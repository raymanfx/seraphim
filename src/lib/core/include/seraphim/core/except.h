/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_CORE_EXCEPT_H
#define SPH_CORE_EXCEPT_H

#include <stdexcept>

/* Used to get file and line information. */
#define SPH_THROW_0(ex) throw ex(__FILE__, __LINE__)
#define SPH_THROW_1(ex, msg) throw ex(__FILE__, __LINE__, msg)
#define SPH_THROW_X(x, ex, msg, FUNC, ...) FUNC
#define SPH_THROW(...)                                                                             \
    SPH_THROW_X(, ##__VA_ARGS__, SPH_THROW_1(__VA_ARGS__), SPH_THROW_0(__VA_ARGS__))

namespace sph {
namespace core {

/**
 * @brief Top level exception.
 *
 * Use this as base class to implement specific exception classes.
 * Template logic is used to allow inheriting from a type which itself inherits from
 * std::exception, such as std::runtime_error.
 */
template <class Base> class Exception : public Base {
public:
    Exception() : Base("") {}

    /**
     * @brief Exception with a message.
     * @param msg The string message.
     */
    explicit Exception(const std::string &msg) : Base(msg) {}

    /**
     * @brief Exception with a message.
     * @param msg The char buffer message.
     */
    explicit Exception(const char *msg) : Base(msg) {}

    /**
     * @brief Exception with file name and line number.
     * @param file The file name where the exception was thrown.
     * @param line The line number where the exception was thrown.
     * @param msg The string message.
     */
    Exception(const std::string &file, size_t line)
        : Base(file + std::string(":") + std::to_string(line)) {}

    /**
     * @brief Exception with file name, line number and a message.
     * @param file The file name where the exception was thrown.
     * @param line The line number where the exception was thrown.
     * @param msg The string message.
     */
    Exception(const std::string &file, size_t line, const std::string &msg)
        : Base(file + std::string(":") + std::to_string(line) + std::string(" ") + msg) {}
};

/**
 * @brief Runtime exception.
 *
 * Throw this exception to report errors (presumably) detectable only when the program runs.
 */
class RuntimeException : public Exception<std::runtime_error> {
public:
    RuntimeException() : Exception("Runtime Error") {}
    using Exception::Exception;
};

/**
 * @brief Logic exception.
 *
 * Throw this exception to report errors (presumably) detectable before the program runs.
 * An example would be violation of logical preconditions which cannot directly be connected to the
 * arguments which were passed to a function.
 */
class LogicException : public Exception<std::logic_error> {
    LogicException() : Exception("Logic Error") {}
    using Exception::Exception;
};

/**
 * @brief Argument exception.
 *
 * Throw this exception to report errors directly related to the caller using wrong arguments to
 * call a function.
 */
class InvalidArgumentException : public Exception<std::invalid_argument> {
    InvalidArgumentException() : Exception("Invalid Argument") {}
    using Exception::Exception;
};

/**
 * @brief Timeout exception.
 *
 * Throw this exception to report timeouts e.g. in nonblocking I/O calls.
 */
class TimeoutException : public RuntimeException {
    TimeoutException() : RuntimeException("Timeout") {}
    using RuntimeException::RuntimeException;
};

} // namespace core
} // namespace sph

#endif // SPH_CORE_EXCEPT_H
