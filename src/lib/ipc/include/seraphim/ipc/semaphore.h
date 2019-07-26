/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_IPC_SHM_SHIM
#define SPH_IPC_SHM_SHIM

#ifdef __APPLE__
#include <dispatch/dispatch.h>
#elif defined(__linux__)
#include <semaphore.h>
#else
#error no semaphore implementation available
#endif

namespace sph {
namespace ipc {

#ifdef __APPLE__
/// Grand Central Dispatch dispatch_semaphore_t
typedef dispatch_semaphore_t sem_t;

/**
 * @brief POSIX sem_init wrapper using Grand Central Dispatch.
 * @param sem The uninitialized semaphore address.
 * @param pshared Ignored.
 * @param value The initial semaphore value.
 * @return 0 on success, -1 on error.
 */
inline int sem_init(sem_t *sem, int pshared, unsigned int value) {
    (void)pshared;
    *sem = dispatch_semaphore_create(value);
    return *sem ? 0 : -1;
}

/**
 * @brief POSIX sem_destroy wrapper using Grand Central Dispatch.
 * @param sem The semaphore address.
 * @return 0
 */
inline int sem_destroy(sem_t *sem) {
    dispatch_release(*sem);
    return 0;
}

/**
 * @brief POSIX sem_wait wrapper using Grand Central Dispatch.
 * @param sem The semaphore address.
 * @return 0 on success, -1 on error.
 */
inline int sem_wait(sem_t *sem) {
    return dispatch_semaphore_wait(*sem, DISPATCH_TIME_FOREVER) == 0 ? 0 : -1;
}

/**
 * @brief POSIX sem_trywait wrapper using Grand Central Dispatch.
 * @param sem The semaphore address.
 * @return 0 on success, -1 on error.
 */
inline int sem_trywait(sem_t *sem) {
    return dispatch_semaphore_wait(*sem, DISPATCH_TIME_NOW) == 0 ? 0 : -1;
}

/**
 * @brief POSIX sem_post wrapper using Grand Central Dispatch.
 * @param sem The semaphore address.
 * @return 0
 */
inline int sem_post(sem_t *sem) {
    dispatch_semaphore_signal(*sem);
    return 0;
}
#elif defined(__linux__)
/// POSIX sem_t
typedef ::sem_t sem_t;

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
#endif

} // namespace ipc
} // namespace sph

#endif // SPH_IPC_SHM_SHIM
