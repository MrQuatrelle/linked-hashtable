#ifndef LHT_DEBUG
#define LHT_DEBUG

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define RED "\033[0;31m"
#define YELLOW "\033[0;33m"
#define GREEN "\033[0;32m"
#define CYAN "\033[0;36m"
#define BLUE "\033[0;34m"
#define RESET "\033[0m"

#define LOG_WRAPPER(mark, ...)                                                 \
    do {                                                                       \
        char buf[2048];                                                        \
        snprintf(buf, 2048, __VA_ARGS__);                                      \
        fprintf(stderr,                                                        \
                mark "  %s:%d :: %s :: %s\n" RESET,                            \
                __FILE__,                                                      \
                __LINE__,                                                      \
                __func__,                                                      \
                buf);                                                          \
    } while (0);

#define WARN(...) LOG_WRAPPER(YELLOW "[WARN]:", __VA_ARGS__)

#define INFO(...) LOG_WRAPPER(GREEN "[INFO]:", __VA_ARGS__)

#define LOG(...) LOG_WRAPPER(CYAN "[LOG]: ", __VA_ARGS__)

#define PANIC(...)                                                             \
    do {                                                                       \
        LOG_WRAPPER(RED "[PANIC]: ", __VA_ARGS__);                             \
        exit(1);                                                               \
    } while (0);

#define OOPS_ERRNO(...)                                                        \
    do {                                                                       \
        char buf[2048];                                                        \
        snprintf(buf, 2048, __VA_ARGS__);                                      \
        fprintf(stderr,                                                        \
                RED "[OOPS]:  %s:%d :: %s :: %s: %s\n" RESET,                  \
                __FILE__,                                                      \
                __LINE__,                                                      \
                __func__,                                                      \
                buf,                                                           \
                strerror(errno));                                              \
    } while (0);

#endif /* !LHT_DEBUG */
