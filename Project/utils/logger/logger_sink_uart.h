
/**
 * @file logger_sink_uart.h
 * @brief Logger sink that forwards logs to AO UART.
 */
#ifndef LOGGER_SINK_UART_H
#define LOGGER_SINK_UART_H

#include "logger.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Sink function to register with log_subscribe(). */
void logger_sink_uart(log_level_t level, const char* msg, size_t len, void* user_ctx);

#ifdef __cplusplus
}
#endif

#endif /* LOGGER_SINK_UART_H */
