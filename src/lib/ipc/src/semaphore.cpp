/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include "seraphim/ipc/semaphore.h"

using namespace sph::ipc;

Semaphore::Semaphore() {
    sem_t *ptr = &m_sem;
    ptr = SEM_FAILED;
    m_name = "";
    m_created = false;
}

Semaphore::~Semaphore() {
    sem_t *ptr = &m_sem;

    if (ptr != SEM_FAILED && m_created) {
        destroy();
    }
}

bool Semaphore::create(const unsigned int &value, const bool &inter_process) {
    sem_t *ptr = &m_sem;

    m_created = true;
#ifdef __APPLE__
    /*
     * Darwin does not support POSIX unnamed semaphores (only GCD).
     * We emulate them here using named semaphores.
     */
    (void)inter_process;
    static unsigned int index = 0;

    ptr = ::sem_open((std::string("/seraphim/") + std::to_string(index)).c_str(), O_CREAT, 0666,
                     value);
    if (ptr == SEM_FAILED) {
        return false;
    }

    index++;
    return true;
#endif

    return ::sem_init(ptr, inter_process ? 1 : 0, value) == 0;
}

bool Semaphore::create(const std::string &name, const unsigned int &value) {
    sem_t *ptr = &m_sem;

    ptr = ::sem_open(name.c_str(), O_CREAT, 0666, value);
    return ptr != SEM_FAILED;
}

bool Semaphore::destroy() {
    sem_t *ptr = &m_sem;
    bool ret = false;

    if (m_name.empty()) {
#ifdef __APPLE__
        /*
         * Darwin does not support POSIX unnamed semaphores (only GCD).
         * We emulate them here using named semaphores.
         */
        return ::sem_unlink(m_name.c_str()) == 0;
#endif
        ret = ::sem_destroy(ptr) == 0;
    } else {
        ret = ::sem_unlink(m_name.c_str()) == 0;
    }

    if (ret) {
        ptr = SEM_FAILED;
    }

    return ret;
}

bool Semaphore::open(sem_t *addr) {
    sem_t *ptr = &m_sem;

    ptr = addr;
    return ptr != SEM_FAILED;
}

bool Semaphore::open(const std::string &name) {
    sem_t *ptr = &m_sem;

    ptr = ::sem_open(name.c_str(), 0);
    return ptr != SEM_FAILED;
}

bool Semaphore::close() {
    sem_t *ptr = &m_sem;
    bool ret;

    ret = ::sem_close(ptr) == 0;
    if (ret) {
        ptr = SEM_FAILED;
    }

    return ret;
}

bool Semaphore::wait() {
    sem_t *ptr = &m_sem;
    return ::sem_wait(ptr);
}

bool Semaphore::trywait() {
    sem_t *ptr = &m_sem;
    return ::sem_trywait(ptr);
}

bool Semaphore::post() {
    sem_t *ptr = &m_sem;
    return ::sem_post(ptr);
}
