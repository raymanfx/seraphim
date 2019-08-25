/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include "seraphim/ipc/semaphore.h"

using namespace sph::ipc;

Semaphore::~Semaphore() {
    if (m_sem != SEM_FAILED && m_created) {
        destroy();
    }
}

bool Semaphore::create(sem_t *sem, const unsigned int &value, const bool &inter_process) {
    m_created = true;
#ifdef __APPLE__
    /*
     * Darwin does not support POSIX unnamed semaphores (only GCD).
     * We emulate them here using named semaphores.
     */
    (void)inter_process;
    static unsigned int index = 0;

    sem = ::sem_open((std::string("/seraphim/") + std::to_string(index)).c_str(), O_CREAT, 0666,
                     value);
    if (sem == SEM_FAILED) {
        return false;
    }

    index++;
    m_sem = sem;
    return true;
#endif

    if (::sem_init(sem, inter_process ? 1 : 0, value) != 0) {
        return false;
    }

    m_sem = sem;
    return true;
}

bool Semaphore::create(const std::string &name, const unsigned int &value) {
    m_sem = ::sem_open(name.c_str(), O_CREAT, 0666, value);
    if (m_sem == SEM_FAILED) {
        return false;
    }

    m_name = name;
    m_created = true;
    return true;
}

bool Semaphore::destroy() {
    bool ret = false;

    if (m_name.empty()) {
#ifdef __APPLE__
        /*
         * Darwin does not support POSIX unnamed semaphores (only GCD).
         * We emulate them here using named semaphores.
         */
        return ::sem_unlink(m_name.c_str()) == 0;
#endif
        ret = ::sem_destroy(m_sem) == 0;
    } else {
        ret = ::sem_unlink(m_name.c_str()) == 0;
    }

    if (ret) {
        m_sem = SEM_FAILED;
    }

    return ret;
}

bool Semaphore::open(sem_t *sem) {
    m_sem = sem;
    return m_sem != SEM_FAILED;
}

bool Semaphore::open(const std::string &name) {
    m_sem = ::sem_open(name.c_str(), 0);
    return m_sem != SEM_FAILED;
}

bool Semaphore::close() {
    if (::sem_close(m_sem) != 0) {
        return false;
    }

    m_sem = SEM_FAILED;
    return true;
}

bool Semaphore::wait() {
    return ::sem_wait(m_sem) == 0;
}

bool Semaphore::trywait() {
    return ::sem_trywait(m_sem) == 0;
}

bool Semaphore::post() {
    return ::sem_post(m_sem) == 0;
}
