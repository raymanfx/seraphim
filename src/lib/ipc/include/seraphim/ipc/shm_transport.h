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
 * @brief Shared memory message transport.
 *
 * This class uses POSIX shared memory to exchange messages between two or more readers and
 * writers. At the moment, it behaves like a very simple bidirectional pipe.
 */
class SharedMemoryTransport : public Transport {
public:
    /**
     * @brief Shared memory message transport.
     */
    SharedMemoryTransport() = default;

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
    bool create(const std::string &name, long size);

    /**
     * @brief Remove the shared memory region created by this instance.
     * @return true on success, false otherwise.
     */
    bool remove();

    void set_rx_timeout(int ms) override { m_rx_timeout = ms; }
    void set_tx_timeout(int ms) override { m_tx_timeout = ms; }

    void receive(Seraphim::Message &msg) override;
    void send(const Seraphim::Message &msg) override;

    /**
     * @brief Actors perform read/write operations within the shared memory segment (MessageStore).
     */
    struct MessageStoreActors {
        /// The current number of active actors.
        /// An actor must increase this number when opening the message store and decrease it
        /// once he wishes to no longer use it (aka close it).
        unsigned int num_actors = 1;
        /// Resource locking for the @ref num_actors field. Must be acquired before any operation
        /// is performed on the field.
        sem_t sem;
    };

    /**
     * @brief Message type.
     */
    enum MessageStoreType {
        MESSAGE_TYPE_UNKNOWN = 0,
        /// Corresponds to a Seraphim::Message request instance.
        MESSAGE_TYPE_REQUEST,
        /// Corresponds to a Seraphim::Message response instance.
        MESSAGE_TYPE_RESPONSE
    };

    /**
     * @brief Messages are objects used for reading/writing information within the MessageStore.
     */
    struct MessageStoreMessage {
        /// Source actor ID.
        unsigned int source;
        /// Destination actor ID.
        unsigned int destination;
        /// Type of the message, allows for multiple concurrent clients talking to a server.
        int type;
        /// Current message size
        int size = 0;
        /// Resource locking for the message. Must be acquired before performing any read/write
        /// operation on the message segment.
        sem_t sem;
    };

    /**
     * @brief Shared memory area layout.
     *
     * Facilitates RX/TX with more than two peers writing and reading simultaneously by keeping
     * track of read and write stats and holding an unnamed semaphore that must be acquired before
     * writing to the shared memory area.
     */
    struct MessageStore {
        MessageStoreActors actors;
        MessageStoreMessage msg;
    };

private:
    /**
     * @brief Map a shared memory region.
     * @param size The size of the memory region.
     * @return true on success, false otherwise.
     */
    bool map(size_t size);
    /**
     * @brief Unmap a shared memory region.
     * @return true on success, false otherwise.
     */
    bool unmap();

    std::string m_name;
    int m_fd = -1;
    size_t m_size = 0;
    int m_rx_timeout = 0;
    int m_tx_timeout = 0;
    bool m_created = false;

    /// Unique identifier of this instance.
    unsigned int m_actor_id;

    /// The mapped MessageStore object.
    MessageStore *m_msgstore = nullptr;

    /// semaphores for inter-process resource locking
    Semaphore m_actor_sem;
    Semaphore m_msg_sem;

    /// Type of the message that was sent by this instance
    int m_last_msg_type = -1;
};

} // namespace ipc
} // namespace sph

#endif // SPH_IPC_SHM_TRANSPORT_H
