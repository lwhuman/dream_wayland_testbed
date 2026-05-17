#ifndef LOG_H
#define LOG_H

#include <stdio.h>

#ifdef DEBUG
#define debug(...)                                             \
    do {                                                       \
        fprintf(stderr, "[DEBUG][%s:%d]", __FILE__, __LINE__); \
        fprintf(stderr, __VA_ARGS__);                          \
        fputc('\n', stderr);                                   \
    } while (0)

#define ASSERT(x)          \
    do {                   \
        if (!(x)) abort(); \
    } while (0)
#else
#define debug(...) ((void)0)
#define ASSERT(x)  ((void)0)
#endif // DEBUG

#ifdef VERBOSE
#define verbose(...)                  \
    do {                              \
        fprintf(stderr, __VA_ARGS__); \
        fputc('\n', stderr);          \
    } while (0)
#else
#define verbose(...) ((void)0)
#endif // VERBOSE

#define ERROR(...)                    \
    do {                              \
        fprintf(stderr, "[ERROR]: "); \
        fprintf(stderr, __VA_ARGS__); \
        fputc('\n', stderr);          \
    } while (0)

#endif // LOG_H
