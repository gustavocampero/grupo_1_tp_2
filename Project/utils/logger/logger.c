
/**
 * @file logger.c
 * @brief Implementation of a lightweight pub/sub logger.
 */
#include "logger.h"
#include <stdio.h>
#include <string.h>

#if LOG_USE_FREERTOS
  #include "FreeRTOS.h"
  #include "semphr.h"
  static SemaphoreHandle_t s_log_mtx = NULL;
  static inline void lock(void){ if (s_log_mtx) xSemaphoreTake(s_log_mtx, portMAX_DELAY); }
  static inline void unlock(void){ if (s_log_mtx) xSemaphoreGive(s_log_mtx); }
#else
  static inline void lock(void){}
  static inline void unlock(void){}
#endif

typedef struct {
    log_sink_fn fn;
    log_level_t min_level;
    void* user;
    int in_use;
} subscriber_t;

static subscriber_t s_subs[LOG_MAX_SUBSCRIBERS];
static char s_buf[LOG_MAX_MESSAGE_LENGTH];

void log_init(void)
{
    memset(s_subs, 0, sizeof(s_subs));
#if LOG_USE_FREERTOS
    if (!s_log_mtx) s_log_mtx = xSemaphoreCreateMutex();
#endif
}

int log_subscribe(log_sink_fn fn, log_level_t min_level, void* user_ctx)
{
    if (!fn) return -1;
    lock();
    int h = -1;
    for (int i=0;i<LOG_MAX_SUBSCRIBERS;i++){
        if (!s_subs[i].in_use){
            s_subs[i].fn = fn; s_subs[i].min_level = min_level;
            s_subs[i].user = user_ctx; s_subs[i].in_use = 1;
            h = i; break;
        }
    }
    unlock();
    return h;
}

int log_unsubscribe(int handle)
{
    if (handle < 0 || handle >= LOG_MAX_SUBSCRIBERS) return -1;
    lock();
    s_subs[handle].in_use = 0;
    s_subs[handle].fn = NULL;
    s_subs[handle].user = NULL;
    s_subs[handle].min_level = LOG_NONE;
    unlock();
    return 0;
}

static size_t format_prefix(char* dst, size_t cap, log_level_t level)
{
    size_t n = 0;
#if LOG_INCLUDE_TIMESTAMP
    n += (size_t)snprintf(dst+n, (cap>n?cap-n:0), "[%10lu] ", (unsigned long)log_get_timestamp_ms());
#endif
    n += (size_t)snprintf(dst+n, (cap>n?cap-n:0), "[%s] ", log_level_str(level));
    return (n>cap?cap:n);
}

size_t log_vprintf(log_level_t level, const char* fmt, va_list ap)
{
    if (level < LOG_DEBUG || level > LOG_ERROR) return 0;
    lock();
    size_t n = format_prefix(s_buf, sizeof(s_buf), level);
    int wrote = vsnprintf(s_buf+n, sizeof(s_buf)-n, fmt, ap);
    if (wrote < 0) wrote = 0;
    n += (size_t)wrote;
    if (n >= sizeof(s_buf)) n = sizeof(s_buf)-1;
    s_buf[n] = '\0';

    for (int i=0;i<LOG_MAX_SUBSCRIBERS;i++){
        if (s_subs[i].in_use && s_subs[i].fn && level >= s_subs[i].min_level){
            s_subs[i].fn(level, s_buf, n, s_subs[i].user);
        }
    }
    unlock();
    return n;
}

size_t log_printf(log_level_t level, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    size_t n = log_vprintf(level, fmt, ap);
    va_end(ap);
    return n;
}

const char* log_level_str(log_level_t level)
{
    switch (level){
        case LOG_DEBUG: return "DEBUG";
        case LOG_INFO:  return "INFO";
        case LOG_WARN:  return "WARN";
        case LOG_ERROR: return "ERROR";
        default:        return "?";
    }
}

__attribute__((weak)) uint32_t log_get_timestamp_ms(void){ return 0u; }
