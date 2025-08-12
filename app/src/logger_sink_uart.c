
/**
 * @file logger_sink_uart.c
 * @brief Implementation of a logger sink that forwards to AO UART.
 */
#include "logger_sink_uart.h"
#include "ao_uart.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <string.h>

static SemaphoreHandle_t logger_mutex = NULL;

void logger_sink_uart_init(void)
{
    logger_mutex = xSemaphoreCreateMutex();
    configASSERT(logger_mutex != NULL);
}

void logger_sink_uart(log_level_t level, const char* msg, size_t len, void* user_ctx)
{
    (void)level; (void)user_ctx;
    
    // Create a buffer for the complete message (msg + newline)
    static char complete_msg[512];  // Static buffer to avoid stack issues
    
    if (logger_mutex != NULL) {
        // Take mutex to ensure atomic transmission
        if (xSemaphoreTake(logger_mutex, pdMS_TO_TICKS(500)) == pdTRUE) {
            // Copy message and add newline in one buffer
            size_t copy_len = (len < sizeof(complete_msg) - 3) ? len : (sizeof(complete_msg) - 3);
            memcpy(complete_msg, msg, copy_len);
            complete_msg[copy_len] = '\r';
            complete_msg[copy_len + 1] = '\n';
            complete_msg[copy_len + 2] = '\0';
            
            // Send as one atomic operation
            (void)ao_uart_post((const uint8_t*)complete_msg, copy_len + 2);
            
            xSemaphoreGive(logger_mutex);
        } else {
            // Mutex timeout - fallback
            (void)ao_uart_post((const uint8_t*)msg, len);
            (void)ao_uart_post((const uint8_t*)"\r\n", 2);
        }
    } else {
        // Fallback if mutex not initialized
        (void)ao_uart_post((const uint8_t*)msg, len);
        (void)ao_uart_post((const uint8_t*)"\r\n", 2);
    }
}
