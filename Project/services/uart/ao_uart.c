
/**
 * @file ao_uart.c
 * @brief Implementation of AO UART: DMA TX + StreamBuffer + FreeRTOS.
 */
#include "ao_uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"
#include "port_uart.h"

#ifndef MIN
#  define MIN(a,b) (( (a) < (b) ) ? (a) : (b))
#endif

static TaskHandle_t         s_task = NULL;
static StreamBufferHandle_t s_tx_stream = NULL;
static volatile bool        s_tx_inflight = false;

static void ao_uart_task(void* arg);

bool ao_uart_create(void)
{
    if (!s_tx_stream) {
        s_tx_stream = xStreamBufferCreate(AO_UART_STREAM_CAPACITY, 1 /* trigger */);
        if (!s_tx_stream) return false;
    }
    if (!s_task) {
        BaseType_t ok = xTaskCreate(ao_uart_task, AO_UART_TASK_NAME,
                                    AO_UART_TASK_STACK, NULL,
                                    AO_UART_TASK_PRIO, &s_task);
        if (ok != pdPASS) return false;
    }
    return true;
}

size_t ao_uart_post(const uint8_t* data, size_t len)
{
    if (!data || len == 0 || !s_tx_stream) return 0;
    size_t wrote = xStreamBufferSend(s_tx_stream, data, len, 0);
    /* Poke AO in case it is waiting */
    if (wrote && s_task) xTaskNotifyGive(s_task);
    return wrote;
}

size_t ao_uart_post_str(const char* s)
{
    if (!s) return 0;
    size_t n = 0; while (s[n] != '\0') ++n;
    return ao_uart_post((const uint8_t*)s, n);
}

bool ao_uart_flush(uint32_t timeout_ticks)
{
    TickType_t start = xTaskGetTickCount();
    for (;;) {
        size_t pending = xStreamBufferBytesAvailable(s_tx_stream);
        size_t used = AO_UART_STREAM_CAPACITY - pending;
        if (used == 0 && !s_tx_inflight) return true;
        if ((xTaskGetTickCount() - start) >= timeout_ticks) return false;
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void ao_uart_on_tx_complete_isr(void)
{
    BaseType_t xHigher = pdFALSE;
    s_tx_inflight = false;
    if (s_task) vTaskNotifyGiveFromISR(s_task, &xHigher);
    portYIELD_FROM_ISR(xHigher);
}

static void ao_uart_task(void* arg)
{
    (void)arg;
    uint8_t chunk[AO_UART_DMA_CHUNK_MAX];

    for (;;) {
        /* Try receive some bytes; if none, block until notified (producer or ISR). */
        size_t n = xStreamBufferReceive(s_tx_stream, chunk, sizeof(chunk), pdMS_TO_TICKS(10));
        if (n == 0) {
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            continue;
        }

        s_tx_inflight = true;
        port_uart_dma_start(chunk, n);

        /* Wait until DMA complete ISR wakes us */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        /* Loop and drain more */
    }
}
