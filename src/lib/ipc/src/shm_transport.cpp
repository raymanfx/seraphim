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

#include "seraphim/ipc/shm_transport.h"

using namespace sph::ipc;

SharedMemoryTransport::SharedMemoryTransport() {
    m_fd = -1;
    m_addr = nullptr;
    m_size = 0;
    m_timeout = 0;
    m_id = 0;
    m_created = false;
}

SharedMemoryTransport::~SharedMemoryTransport() {
    if (m_addr != nullptr) {
        if (m_created) {
            m_sem.destroy();
        }
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
    MessageStore *msgstore;

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

    msgstore = reinterpret_cast<MessageStore *>(m_addr);
    if (!m_sem.open(&msgstore->sem)) {
        unmap();
        return false;
    }

    return true;
}

bool SharedMemoryTransport::close() {
    return ::close(m_fd) == 0;
}

bool SharedMemoryTransport::create(const std::string &name, const long &size) {
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

    MessageStore *msgstore = reinterpret_cast<MessageStore *>(m_addr);
    msgstore->id = 1;
    msgstore->status = MESSAGE_READ;
    msgstore->msg_size = 0;
    // create a semaphore for the segment
    if (!m_sem.create(&msgstore->sem, 1)) {
        remove();
        return false;
    }

    m_name = name;
    m_created = true;
    return true;
}

bool SharedMemoryTransport::remove() {
    int ret;

    ret = shm_unlink(m_name.c_str());
    m_fd = -1;

    return ret == 0;
}

bool SharedMemoryTransport::map(const size_t &size) {
    m_addr = mmap(nullptr, size, PROT_WRITE, MAP_SHARED, m_fd, 0);
    if (m_addr != MAP_FAILED) {
        m_size = size;
    }

    return m_addr != MAP_FAILED;
}

bool SharedMemoryTransport::unmap() {
    int ret;

    ret = munmap(m_addr, m_size);
    m_addr = nullptr;
    m_size = 0;

    return ret == 0;
}

ITransport::IOResult SharedMemoryTransport::recv(Seraphim::Message &msg) {
    int elapsed_ms = 0;
    MessageStore *msgstore = reinterpret_cast<MessageStore *>(m_addr);

    if (m_id == 0) {
        m_id = msgstore->id;
    }

    // block until a message is available
    while (m_id == msgstore->id || msgstore->status != MessageStoreStatus::MESSAGE_UNREAD) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        elapsed_ms++;
        if (elapsed_ms < 0) {
            elapsed_ms = 1;
        }

        if (m_timeout > 0 && elapsed_ms >= m_timeout) {
            return ITransport::IOResult::TIMEOUT;
        }
    }

    // read the message
    if (!msg.ParseFromArray(static_cast<char *>(m_addr) + sizeof(MessageStore),
                            msgstore->msg_size)) {
        return ITransport::IOResult::ERROR;
    }

    msgstore->status = MessageStoreStatus::MESSAGE_READ;
    return ITransport::IOResult::OK;
}

ITransport::IOResult SharedMemoryTransport::send(const Seraphim::Message &msg) {
    int elapsed_ms = 0;
    MessageStore *msgstore = reinterpret_cast<MessageStore *>(m_addr);

    if (msg.ByteSizeLong() > m_size - sizeof(MessageStore::msg_size)) {
        return ITransport::IOResult::ERROR;
    }

    // block until a message was read and can thus be overwritten
    while ((!m_sem.trywait()) || (msgstore->status != MessageStoreStatus::MESSAGE_READ)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        elapsed_ms++;
        if (elapsed_ms < 0) {
            elapsed_ms = 1;
        }

        if (m_timeout > 0 && elapsed_ms >= m_timeout) {
            return ITransport::IOResult::TIMEOUT;
        }
    }

    msg.SerializeToArray(static_cast<char *>(m_addr) + sizeof(MessageStore), msg.ByteSize());

    // prevent us from reading our own messages in recv() by keeping track of
    // who sent a message and waiting for the next one
    msgstore->id++;
    if (msgstore->id == 0) {
        msgstore->id = 1;
    }
    m_id = msgstore->id;
    msgstore->msg_size = msg.ByteSize();
    msgstore->status = MessageStoreStatus::MESSAGE_UNREAD;
    m_sem.post();

    return ITransport::IOResult::OK;
}
