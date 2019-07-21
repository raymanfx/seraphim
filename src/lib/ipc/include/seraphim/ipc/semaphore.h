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

/// POSIX sem_t
typedef ::sem_t sem_t;

#ifdef __APPLE__
/**
 * @brief POSIX sem_init wrapper implemented using sem_open.
 * @param sem The uninitialized semaphore address.
 * @param pshared Whether to share the semaphore between threads (0) or processes (nonzero).
 * @param value The initial semaphore value.
 * @return 0 on success, -1 on error.
 */
inline int sem_init(sem_t *sem, int pshared, unsigned int value) {
    (void)pshared;
    static unsigned int index = 0;
    sem = ::sem_open((std::string("/seraphim/") + std::to_string(index)).c_str(), O_CREAT, 0666,
                     value);
    if (sem == SEM_FAILED) {
        return -1;
    }

    index++;
    return 0;
}

/**
 * @brief POSIX sem_destroy wrapper implemented using sem_close.
 * @param sem The semaphore address.
 * @return 0 on success, -1 on error.
 */
inline int sem_destroy(sem_t *sem) {
    return ::sem_close(sem);
}
#elif defined(__linux__)
/**
 * @brief POSIX sem_init.
 * @param sem The uninitialized semaphore address.
 * @param pshared Whether to share the semaphore between threads (0) or processes (nonzero).
 * @param value The initial semaphore value.
 * @return 0 on success, -1 on error.
 */
inline int sem_init(sem_t *sem, int pshared, unsigned int value) {
    return ::sem_init(sem, pshared, value);
}

/**
 * @brief POSIX sem_destroy.
 * @param sem The semaphore address.
 * @return 0 on success, -1 on error.
 */
inline int sem_destroy(sem_t *sem) {
    return ::sem_destroy(sem);
}
#endif

/**
 * @brief POSIX sem_open.
 * @param sem The semaphore address.
 * @param oflag Open mode flags (e.g. O_CREAT to create a new semaphore).
 * @return 0 on success, -1 on error.
 */
inline sem_t *sem_open(const char *name, int oflag) {
    return ::sem_open(name, oflag);
}

/**
 * @brief POSIX sem_open.
 * @param sem The semaphore address.
 * @param oflag Open mode flags (e.g. O_CREAT to create a new semaphore).
 * @param mode Permission mask for the new semaphore.
 * @value The initial semaphore value.
 * @return 0 on success, -1 on error.
 */
inline sem_t *sem_open(const char *name, int oflag, mode_t mode, unsigned int value) {
    return ::sem_open(name, oflag, mode, value);
}

/**
 * @brief POSIX sem_close.
 * @param sem The semaphore address.
 * @return 0 on success, -1 on error.
 */
inline int sem_close(sem_t *sem) {
    return ::sem_close(sem);
}

/**
 * @brief POSIX sem_wait.
 * @param sem The semaphore address.
 * @return 0 on success, -1 on error.
 */
inline int sem_wait(sem_t *sem) {
    return ::sem_wait(sem);
}

/**
 * @brief POSIX sem_trywait.
 * @param sem The semaphore address.
 * @return 0 on success, -1 on error.
 */
inline int sem_trywait(sem_t *sem) {
    return ::sem_trywait(sem);
}

/**
 * @brief POSIX sem_post.
 * @param sem The semaphore address.
 * @return 0 on success, -1 on error.
 */
inline int sem_post(sem_t *sem) {
    return ::sem_post(sem);
}

} // namespace ipc
} // namespace sph

#endif // SPH_IPC_SHM_SHIM
