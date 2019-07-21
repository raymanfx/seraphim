/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_IPC_SHM_SHIM
#define SPH_IPC_SHM_SHIM

#include <fcntl.h>
#include <string>
#if defined(__APPLE__) || defined(__unix__)
#include <semaphore.h>
#else
#error no semaphore implementation available
#endif

namespace sph {
namespace ipc {

class Semaphore {
public:
    Semaphore();
    ~Semaphore();

    /**
     * @brief POSIX sem_init (unnamed semaphore creation).
     * @param value The initial semaphore value.
     * @param inter_process Whether to share the semaphore between threads (false) or processes (true).
     * @return True on success, false otherwise.
     */
    bool create(const unsigned int &value, const bool &inter_process = true);

    /**
     * @brief POSIX sem_open (named semaphore creation).
     * @param name The semaphore name.
     * @param value The initial semaphore value.
     * @return True on success, false otherwise.
     */
    bool create(const std::string &name, const unsigned int &value);

    /**
     * @brief Open an existing semaphore at a given address.
     * @param name The memory address of the semaphore.
     * @return True on success, false otherwise.
     */
    bool open(sem_t *addr);

    /**
     * @brief Open an existing semaphore at a given address.
     * @param name The memory address of the semaphore.
     * @return True on success, false otherwise.
     */
    bool open(const std::string &name);

    /**
     * @brief POSIX sem_destroy/sem_close (unnamed/named semaphore destruction).
     * @return True on success, false otherwise.
     */
    bool close();

    /**
     * @brief POSIX sem_wait.
     * @return True on success, false otherwise.
     */
    bool wait();

    /**
     * @brief POSIX sem_trywait.
     * @return True on success, false otherwise.
     */
    bool trywait();

    /**
     * @brief POSIX sem_post.
     * @return True on success, false otherwise.
     */
    bool post();

    /**
     * @brief Get the memory address of the semaphore object.
     * @return Memory address. SEM_FAILED if the semaphore is invalid.
     */
    sem_t *ptr() { return &m_sem; }

private:
    /// POSIX sem_t
    sem_t m_sem;

    /// name of the semaphore (empty for unnamed ones)
    std::string m_name;
};

} // namespace ipc
} // namespace sph

#endif // SPH_IPC_SHM_SHIM
