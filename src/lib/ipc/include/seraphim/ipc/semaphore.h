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

/**
 * @brief Semaphore (shared resource lock).
 *
 * This class uses POSIX semaphores provide locks that are shared between threads and/or processes.
 * This way, shared memory can be protected so one client does not overwrite data of another one
 * which also happens to be writing to it at that moment.
 */
class Semaphore {
public:
    /**
     * @brief Semaphore wrapper, the constructor does nothing in itself.
     */
    Semaphore() = default;

    /**
     * @brief Semaphore destructor.
     * Closes the semaphore if it was opened.
     * Removes the semaphore if it was created.
     */
    ~Semaphore();

    /**
     * @brief POSIX sem_init (unnamed semaphore creation).
     * @param sem The semaphore location in a shared memory segment.
     * @param value The initial semaphore value.
     * @param inter_process Whether to share the semaphore between threads (false) or processes
     * (true).
     * @return True on success, false otherwise.
     */
    bool create(sem_t *sem, const unsigned int &value, const bool &inter_process = true);

    /**
     * @brief POSIX sem_open (named semaphore creation).
     * @param name The semaphore name.
     * @param value The initial semaphore value.
     * @return True on success, false otherwise.
     */
    bool create(const std::string &name, const unsigned int &value);

    /**
     * @brief POSIX sem_destroy/sem_unlink (unnamed/named semaphore destruction).
     * @return True on success, false otherwise.
     */
    bool destroy();

    /**
     * @brief Open an existing semaphore at a given address.
     * @param sem The memory address of the semaphore.
     * @return True on success, false otherwise.
     */
    bool open(sem_t *sem);

    /**
     * @brief Open an existing semaphore at a given address.
     * @param name The memory address of the semaphore.
     * @return True on success, false otherwise.
     */
    bool open(const std::string &name);

    /**
-    * @brief POSIX sem_close (call this when you are done using the named semaphore).
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

private:
    /// POSIX sem_t
    sem_t *m_sem = SEM_FAILED;

    /// name of the semaphore (empty for unnamed ones)
    std::string m_name = "";

    /// whether this instance has created the underlying semaphore object
    bool m_created = false;
};

} // namespace ipc
} // namespace sph

#endif // SPH_IPC_SHM_SHIM
