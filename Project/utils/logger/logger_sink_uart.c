
/**
 * @file logger_sink_uart.c
 * @brief Implementation of a logger sink that forwards to AO UART.
 */
#include "logger_sink_uart.h"
#include "ao_uart.h"

void logger_sink_uart(log_level_t level, const char* msg, size_t len, void* user_ctx)
{
    (void)level; (void)user_ctx;
    (void)ao_uart_post((const uint8_t*)msg, len);
}
