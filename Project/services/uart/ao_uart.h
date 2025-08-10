
/**
 * @file ao_uart.h
 * @brief Active Object (AO) that owns UART TX using DMA + FreeRTOS.
 *
 * Overview
 * --------
 * - Single task owns the UART and serializes all TX.
 * - Producers call ao_uart_post()/ao_uart_post_str() (non-blocking):
 *   data goes into an internal StreamBuffer.
 * - The AO drains in chunks and starts DMA; the TX-complete ISR wakes it.
 *
 * Notes
 * -----
 * - Not ISR-safe. Provide your own lightweight FromISR path if needed.
 * - This AO handles TX only. RX can be added separately if required.
 */
#ifndef AO_UART_H
#define AO_UART_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------- Configuration (override at compile time if needed) ------- */
#ifndef AO_UART_TASK_NAME
#  define AO_UART_TASK_NAME       "ao_uart"
#endif

#ifndef AO_UART_TASK_STACK
#  define AO_UART_TASK_STACK      640   /* words (depends on port) */
#endif

#ifndef AO_UART_TASK_PRIO
#  define AO_UART_TASK_PRIO       2
#endif

#ifndef AO_UART_STREAM_CAPACITY
#  define AO_UART_STREAM_CAPACITY 1024  /* bytes */
#endif

#ifndef AO_UART_DMA_CHUNK_MAX
#  define AO_UART_DMA_CHUNK_MAX   256   /* bytes per DMA transfer */
#endif

/* ---------------- API ------------------------------------------------------ */

/** Create/start AO and internal stream. Call after port_uart_hw_init(). */
bool ao_uart_create(void);

/** Enqueue data for TX (non-blocking). Returns bytes accepted (<= len). */
size_t ao_uart_post(const uint8_t* data, size_t len);

/** Enqueue C-string (without trailing '\0'). */
size_t ao_uart_post_str(const char* s);

/** Block until buffered data is transmitted (optional). */
bool ao_uart_flush(uint32_t timeout_ticks);

/** Called by UART DMA TX-complete ISR to wake the AO. */
void ao_uart_on_tx_complete_isr(void);

#ifdef __cplusplus
}
#endif

#endif /* AO_UART_H */
