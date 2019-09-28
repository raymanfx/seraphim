/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <chrono>
#include <cstring>
#include <poll.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>

#include "seraphim/core/except.h"
#include "seraphim/ipc/shm_transport.h"

using namespace sph::core;
using namespace sph::ipc;

SharedMemoryTransport::~SharedMemoryTransport() {
    if (m_msgstore != nullptr) {
        unmap();
    }

    if (m_created && m_fd > -1) {
        remove();
    } else if (m_fd > -1) {
        close();
    }
}

bool SharedMemoryTransport::open(const std::string &name) {
    struct stat shm_stat;

    m_fd = shm_open(name.c_str(), O_RDWR, S_IRUSR | S_IWUSR);
    if (m_fd == -1) {
        return false;
    }

    if (fstat(m_fd, &shm_stat) == -1) {
        return false;
    }

    if (!map(static_cast<size_t>(shm_stat.st_size))) {
        return false;
    }

    if (!m_actor_sem.open(&m_msgstore->actors.sem)) {
        unmap();
        return false;
    }

    if (!m_msg_sem.open(&m_msgstore->actors.sem)) {
        unmap();
        return false;
    }

    if (!m_actor_sem.wait()) {
        unmap();
        return false;
    }

    m_msgstore->actors.num_actors++;
    m_actor_id = m_msgstore->actors.num_actors;
    if (!m_actor_sem.post()) {
        unmap();
        return false;
    }

    return true;
}

bool SharedMemoryTransport::close() {
    return ::close(m_fd) == 0;
}

bool SharedMemoryTransport::create(const std::string &name, long size) {
    struct stat shm_stat;

    // shared memory segment size must at least be the size of the controlling structure
    if (static_cast<size_t>(size) < sizeof(MessageStore)) {
        return false;
    }

    m_fd = shm_open(name.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (m_fd == -1) {
        return false;
    }

    if (fstat(m_fd, &shm_stat) == -1) {
        return false;
    }

    // on Linux, ftruncate() can always be called, but on macOS it fails with -EINVAL after the
    // first time it has been called, so check the segment size before truncating
    if (shm_stat.st_size == 0 && ftruncate(m_fd, size) == -1) {
        remove();
        return false;
    }

    if (!map(static_cast<size_t>(size))) {
        remove();
        return false;
    }

    m_msgstore->actors.num_actors = 1;
    m_msgstore->msg.source = 0;
    m_msgstore->msg.destination = 0;
    m_msgstore->msg.type = MESSAGE_TYPE_UNKNOWN;
    m_msgstore->msg.size = 0;
    // create semaphores for the segment
    if (!m_actor_sem.create(&m_msgstore->actors.sem, 1) ||
        !m_msg_sem.create(&m_msgstore->msg.sem, 1)) {
        remove();
        return false;
    }

    m_name = name;
    m_created = true;
    m_actor_id = 1;
    return true;
}

bool SharedMemoryTransport::remove() {
    int ret;

    ret = shm_unlink(m_name.c_str());
    m_fd = -1;

    return ret == 0;
}

bool SharedMemoryTransport::map(size_t size) {
    void *addr;

    addr = mmap(nullptr, size, PROT_WRITE, MAP_SHARED, m_fd, 0);
    if (addr == MAP_FAILED) {
        return false;
    }

    m_size = size;
    m_msgstore = reinterpret_cast<MessageStore *>(addr);
    return true;
}

bool SharedMemoryTransport::unmap() {
    bool ret;

    m_size = 0;
    if (!m_msgstore) {
        return true;
    }

    if (m_actor_sem.wait()) {
        if (m_msgstore->actors.num_actors > 0) {
            m_msgstore->actors.num_actors--;
        }
        m_actor_sem.post();
    }

    ret = munmap(m_msgstore, m_size) == 0;
    m_msgstore = nullptr;
    m_size = 0;

    return ret;
}

void SharedMemoryTransport::receive(Seraphim::Message &msg) {
    int elapsed_ms = 0;
    unsigned char *msg_ptr;

    if (!m_msgstore) {
        SPH_THROW(RuntimeException, "Memory region not mapped");
    }

    // block until a message is available
    while (m_msgstore->msg.type == m_last_msg_type || m_msgstore->msg.destination != m_actor_id) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        elapsed_ms++;
        if (elapsed_ms < 0) {
            elapsed_ms = 1;
        }

        if (m_rx_timeout > 0 && elapsed_ms >= m_rx_timeout) {
            SPH_THROW(TimeoutException);
        }
    }

    // read the message
    msg_ptr = reinterpret_cast<unsigned char *>(m_msgstore) + sizeof(MessageStore);
    if (!msg.ParseFromArray(msg_ptr, m_msgstore->msg.size)) {
        SPH_THROW(RuntimeException, "Failed to deserialize message");
    }
}

void SharedMemoryTransport::send(const Seraphim::Message &msg) {
    int elapsed_ms = 0;

    if (!m_msgstore) {
        SPH_THROW(RuntimeException, "Memory region not mapped");
    }

    if (msg.ByteSizeLong() > m_size - sizeof(MessageStore)) {
        SPH_THROW(RuntimeException, std::string("Memory region too small (") +
                                        std::to_string(msg.ByteSizeLong()) + std::string(" < ") +
                                        std::to_string(m_size - sizeof(MessageStore)) +
                                        std::string(")"));
    }

    // block until a message was read and can thus be overwritten
    while (m_msgstore->msg.type == m_last_msg_type || m_msgstore->msg.source == m_actor_id ||
           !m_msg_sem.trywait()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        elapsed_ms++;
        if (elapsed_ms < 0) {
            elapsed_ms = 1;
        }

        if (m_tx_timeout > 0 && elapsed_ms >= m_tx_timeout) {
            SPH_THROW(TimeoutException);
        }
    }

    if (!msg.SerializeToArray(reinterpret_cast<unsigned char *>(m_msgstore) + sizeof(MessageStore),
                              msg.ByteSize())) {
        SPH_THROW(RuntimeException, "Failed to serialize message");
    }

    // prevent us from reading our own messages in recv() by keeping track of
    // who sent a message and waiting for the next one
    m_msgstore->msg.destination = m_actor_id == 1 ? m_msgstore->msg.source : 1;
    m_msgstore->msg.source = m_actor_id;
    m_msgstore->msg.size = msg.ByteSize();

    m_msg_sem.post();

    m_last_msg_type = msg.has_req() ? MESSAGE_TYPE_REQUEST : MESSAGE_TYPE_RESPONSE;
}
