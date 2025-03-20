#ifndef _DEBUG_MACROS_HEADER
#define _DEBUG_MACROS_HEADER

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define RED "\033[0;31m"
#define YELLOW "\033[0;33m"
#define GREEN "\033[0;32m"
#define CYAN "\033[0;36m"
#define BLUE "\033[0;34m"
#define RESET "\033[0m"

/* DEBUG/ASSERTION MACROS */

#define DEBUG_IF_WRAPPER(cond, name, ...)                                      \
    if ((cond)) {                                                              \
        name(__VA_ARGS__);                                                     \
    }

#define DEBUG_WRAPPER(color, mark, format, ...)                                \
    do {                                                                       \
        fprintf(stderr,                                                        \
                color "[" mark "]: %s:%d :: %s :: " format "\n" RESET,         \
                __FILE__,                                                      \
                __LINE__,                                                      \
                __func__,                                                      \
                ##__VA_ARGS__);                                                \
    } while (0);

#define BAIL(retval, ...)                                                      \
    do {                                                                       \
        DEBUG_WRAPPER(YELLOW, "BAIL", ##__VA_ARGS__);                          \
        return retval;                                                         \
    } while (0);

#define BAIL_IF(cond, ...) DEBUG_IF_WRAPPER(cond, BAIL, ##__VA_ARGS__)

#define PANIC(...)                                                             \
    do {                                                                       \
        DEBUG_WRAPPER(RED, "PANIC", ##__VA_ARGS__);                            \
        exit(EXIT_FAILURE);                                                    \
    } while (0);

#define PANIC_IF(cond, ...) DEBUG_IF_WRAPPER(cond, PANIC, ##__VA_ARGS__)

#define PANIC_ERRNO(format, ...)                                               \
    do {                                                                       \
        PANIC(format ": %s\n", ##__VA_ARGS__, strerror(errno));                \
        exit(EXIT_FAILURE);                                                    \
    } while (0);

#define PANIC_ERRNO_IF(cond, ...)                                              \
    DEBUG_IF_WRAPPER(cond, PANIC_ERRNO, ##__VA_ARGS__)

#define LOG_ERRNO(format, ...)                                                 \
    DEBUG_WRAPPER(RED, "ERROR", format ": %s", ##__VA_ARGS__, strerror(errno))

#define LOG_ERRNO_IF(cond, ...) DEBUG_IF_WRAPPER(cond, LOG_ERRNO, ##__VA_ARGS__)

/* Toggleable debug macros */

#ifdef DEBUG_MACROS

#define LOG(...) DEBUG_WRAPPER(CYAN, "LOG", ##__VA_ARGS__);

#define LOG_IF(cond, ...) DEBUG_IF_WRAPPER(cond, LOG, ##__VA_ARGS__)

#define WARN(...) DEBUG_WRAPPER(YELLOW, "WARN", ##__VA_ARGS__)

#define WARN_IF(cond, ...) DEBUG_IF_WRAPPER(cond, WARN, ##__VA_ARGS__)

#else

#define LOG(...)
#define LOG_IF(cond, ...) (void)(cond);
#define WARN(...)
#define WARN_IF(cond, ...) (void)(cond);

#endif /* !DEBUG_MACROS */

#endif /* !_DEBUG_MACROS_HEADER */
