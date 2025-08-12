
/**
 * @file logger.h
 * @brief Lightweight pub/sub logger: format once, fan-out to sinks.
 */
#ifndef LOGGER_H
#define LOGGER_H

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Macro definitions for different log levels
#define LOGGER_DEBUG(...) log_printf(LOG_DEBUG, __VA_ARGS__)
#define LOGGER_INFO(...)  log_printf(LOG_INFO, __VA_ARGS__)
#define LOGGER_WARN(...)  log_printf(LOG_WARN, __VA_ARGS__)
#define LOGGER_ERROR(...) log_printf(LOG_ERROR, __VA_ARGS__)

#ifndef LOG_MAX_SUBSCRIBERS
#  define LOG_MAX_SUBSCRIBERS    4
#endif

#ifndef LOG_MAX_MESSAGE_LENGTH
#  define LOG_MAX_MESSAGE_LENGTH 256
#endif

#ifndef LOG_USE_FREERTOS
#  define LOG_USE_FREERTOS 1
#endif

#ifndef LOG_INCLUDE_TIMESTAMP
#  define LOG_INCLUDE_TIMESTAMP 0
#endif

typedef enum {
    LOG_DEBUG = 0,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_NONE
} log_level_t;

/** Sink callback: must be fast and non-blocking. */
typedef void (*log_sink_fn)(log_level_t level,
                            const char* msg,
                            size_t len,
                            void* user_ctx);

void   log_init(void);
int    log_subscribe(log_sink_fn fn, log_level_t min_level, void* user_ctx);
int    log_unsubscribe(int handle);
size_t log_printf(log_level_t level, const char* fmt, ...);
size_t log_vprintf(log_level_t level, const char* fmt, va_list ap);
const char* log_level_str(log_level_t level);

/* Optional hook (weak): timestamp in ms if LOG_INCLUDE_TIMESTAMP != 0 */
uint32_t log_get_timestamp_ms(void);

#ifdef __cplusplus
}
#endif

#endif /* LOGGER_H */
