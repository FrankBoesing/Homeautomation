#pragma once



#ifndef LOGLEVEL
#define LOGLEVEL -1
#endif

#ifndef LOG_UDP
#define LOG_UDP false
#endif

#ifndef LOG_TCP
#define LOG_TCP false
#endif

#ifndef LOG_DEV
#define LOG_DEV Serial
#endif

#define __LOG(__prefix, lvl, fmt, ...)                                    \
    do                                                                    \
    {                                                                     \
        if (LOGLEVEL >= lvl)                                              \
        {                                                                 \
            LOG_DEV.printf_P(PSTR("[" __prefix "] " fmt), ##__VA_ARGS__); \
        }                                                                 \
    } while (0)

#define LOG(lvl, fmt, ...) __LOG(LOG_PREFIX, lvl, fmt, ##__VA_ARGS__)

#if defined(LOG_UDP) && LOG_UDP
#define LOGUDP(lvl, fmt, ...) __LOG(LOG_PREFIX " UDP", lvl, fmt, ##__VA_ARGS__)
#else
#define LOGUDP(lvl, fmt, ...) \
    do                        \
    {                         \
    } while (0)
#endif

#if defined(LOG_TCP) && LOG_TCP
#define LOGTCP(lvl, fmt, ...) __LOG(LOG_PREFIX " TCP", lvl, fmt, ##__VA_ARGS__)
#else
#define LOGTCP(lvl, fmt, ...) \
    do                        \
    {                         \
    } while (0)
#endif

/******************************************************************************************/

#define PRINTDEF(size) \
    char _buf[size];   \
    size_t _sz = _buf[0] = 0;
#define PRINTPGMSZ(size, fmt, ...)                                                    \
    do                                                                        \
    {                                                                         \
        _sz += snprintf_P(_buf + _sz, size - _sz, fmt, ##__VA_ARGS__); \
    } while (0)
#define PRINTPGM(fmt, ...)                                                    \
    do                                                                        \
    {                                                                         \
        _sz += snprintf_P(_buf + _sz, sizeof _buf - _sz, fmt, ##__VA_ARGS__); \
    } while (0)
#define PRINT(fmt, ...)                     \
    do                                      \
    {                                       \
        PRINTPGM(PSTR(fmt), ##__VA_ARGS__); \
    } while (0)
#define PRINTCHECK()                    \
    {                                   \
        if (_sz > sizeof _buf)         \
            assert(sizeof _buf > _sz); \
    }

/******************************************************************************************/
