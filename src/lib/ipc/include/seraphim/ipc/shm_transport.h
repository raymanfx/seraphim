/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_IPC_SHM_TRANSPORT_H
#define SPH_IPC_SHM_TRANSPORT_H

#include <fcntl.h>
#include <queue>
#include <string>
#include <sys/mman.h>

#include "transport.h"
#include <seraphim/ipc/semaphore.h>

namespace sph {
namespace ipc {

/**
 * @brief Status of the message store.
 */
enum MessageStoreStatus {
    /// A message has been read as the last operation.
    MESSAGE_READ = 0,
    /// A message has been written as the last operation.
    MESSAGE_UNREAD
};

/**
 * @brief Shared memory area layout.
 *
 * Facilitates RX/TX with more than two peers writing and reading simultaneously by keeping track
 * of read and write stats and holding an unnamed semaphore that must be acquired before writing to
 * the shared memory area.
 */
struct MessageStore {
    /// Message id, must be incremented after writing a message.
    unsigned int id;
    /// Message status (see @ref MessageStoreStatus).
    int status;
    /// Current message size
    int msg_size;
    /// Unnamed POSIX semaphore, must be acquired before writing a message.
    sem_t *sem;
};

/**
 * @brief Shared memory message transport.
 *
 * This class uses POSIX shared memory to exchange messages between two or more readers and
 * writers. At the moment, it behaves like a very simple bidirectional pipe.
 */
class SharedMemoryTransport : public ITransport {
public:
    /**
     * @brief Shared memory message transport.
     */
    SharedMemoryTransport();

    /**
     * @brief Shared memory message transport destructor.
     * Unmaps the shared memory region if it was mapped.
     * Removed the shared memory region if it was created.
     */
    ~SharedMemoryTransport() override;

    /**
     * @brief Open a shared memory region by name.
     * @param name The unique name of the file.
     * @return true on success, false otherwise.
     */
    bool open(const std::string &name);

    /**
     * @brief Close a shared memory region and release associated resources.
     * @return true on success, false otherwise.
     */
    bool close();

    /**
     * @brief Create a shared memory region.
     * @param name The unique name of the file to be created.
     * @param size The size of the memory region.
     * @return true on success, false otherwise.
     */
    bool create(const std::string &name, const long &size);

    /**
     * @brief Remove the shared memory region created by this instance.
     * @return true on success, false otherwise.
     */
    bool remove();

    void set_timeout(const int &ms) override { m_timeout = ms; }

    IOResult recv(Seraphim::Message &msg) override;
    IOResult send(const Seraphim::Message &msg) override;

private:
    /**
     * @brief Map a shared memory region.
     * @param size The size of the memory region.
     * @return true on success, false otherwise.
     */
    bool map(const size_t &size);
    /**
     * @brief Unmap a shared memory region.
     * @return true on success, false otherwise.
     */
    bool unmap();

    std::string m_name;
    int m_fd;
    void *m_addr;
    size_t m_size;
    int m_timeout;
    bool m_created;
    unsigned int m_id;

    /// semaphore for inter-process resource locking
    Semaphore m_sem;
};

} // namespace ipc
} // namespace sph

#endif // SPH_IPC_SHM_TRANSPORT_H
