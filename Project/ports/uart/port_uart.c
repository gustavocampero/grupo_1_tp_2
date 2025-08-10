
/**
 * @file port_uart.c
 * @brief STM32 HAL template for UART TX via DMA used by AO UART.
 *
 * How to wire (STM32 HAL + FreeRTOS):
 * -----------------------------------
 * 1) Ensure your CubeMX project initializes the chosen UART + DMA (GPIO, clocks, NVIC).
 * 2) Set PORT_UART_HANDLE to point to your UART handle (e.g., &huart2).
 * 3) Call port_uart_hw_init() early (after HAL_Init, clocks, MX_*_Init).
 * 4) AO UART will call port_uart_dma_start(buf,len). This uses HAL_UART_Transmit_DMA().
 * 5) In HAL_UART_TxCpltCallback(), call ao_uart_on_tx_complete_isr().
 *
 * Notes:
 *  - This is TX-only. RX can be added similarly if needed.
 *  - Make sure DMA TX complete IRQ priority is compatible with FreeRTOS
 *    (<= configMAX_SYSCALL_INTERRUPT_PRIORITY on Cortex-M ports).
 */

#include "port_uart.h"
#include "ao_uart.h"   /* for ao_uart_on_tx_complete_isr() */
#include <string.h>

/* ----------- Include your device HAL here (adjust family as needed) -------- */
#include "stm32f4xx_hal.h"   /* e.g., F4. For other families: stm32g4xx_hal.h, etc. */

/* ----------- Select your UART instance ------------------------------------ */
/* Provide your UART handle from elsewhere (CubeMX-generated) */
extern UART_HandleTypeDef huart2;   /* change if you use a different UART */

#define PORT_UART_HANDLE   (&huart2)

/* ----------- API ----------------------------------------------------------- */
void port_uart_hw_init(void)
{
    /* Typically, CubeMX generates MX_USART2_UART_Init(); call it before this if needed.
       If your project already calls MX_USART2_UART_Init() in main, you can leave
       this function empty or use it to assert the handle is ready. */
    /* Example (uncomment if you expose the MX init):
       MX_USART2_UART_Init();
    */
}

void port_uart_dma_start(const uint8_t* buf, size_t len)
{
    if (!buf || len == 0) return;
    /* HAL UART DMA expects uint16_t length on some families; cast is safe if <= 0xFFFF */
    HAL_UART_Transmit_DMA(PORT_UART_HANDLE, (uint8_t*)buf, (uint16_t)len);
}

/* ----------- HAL Callbacks ------------------------------------------------- */
/* HAL calls this from the DMA TX complete interrupt context.
 * We translate it to AO UART notification. */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == PORT_UART_HANDLE) {
        ao_uart_on_tx_complete_isr();
    }
}

/* Optional: error callback to observe UART errors (framing, overrun, etc.) */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart == PORT_UART_HANDLE) {
        /* You might log an error or reset state if needed. */
        /* (Do not block here) */
    }
}
