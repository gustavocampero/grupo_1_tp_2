
/**
 * @file port_uart.h
 * @brief HW-specific UART hooks used by AO UART (implement for your MCU).
 */
#ifndef PORT_UART_H
#define PORT_UART_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize UART peripheral + DMA + IRQs. Call before ao_uart_create(). */
void port_uart_hw_init(void);

/** Start non-blocking DMA TX of len bytes from buf. Triggers TX-complete IRQ. */
void port_uart_dma_start(const uint8_t* buf, size_t len);

/** In your DMA TX-complete ISR, call: ao_uart_on_tx_complete_isr(); */

#ifdef __cplusplus
}
#endif

#endif /* PORT_UART_H */
