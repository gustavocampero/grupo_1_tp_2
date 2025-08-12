/*
 * Copyright (c) 2023 Sebastian Bedin <sebabedin@gmail.com>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * @author : Sebastian Bedin <sebabedin@gmail.com>
 */

/********************** inclusions *******************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "task_led.h"
#include "main.h"
#include "cmsis_os.h"
#include "board.h"
#include "logger.h"
#include "dwt.h"

/********************** macros and definitions *******************************/

#define TASK_PERIOD_MS_           (1000)

#define QUEUE_LENGTH_            (10)
#define QUEUE_ITEM_SIZE_         (sizeof(ao_led_message_t))

/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/

static QueueHandle_t hqueue;

/********************** external data definition *****************************/

/********************** internal functions definition ************************/

static const char* ledColorToStr(ao_led_color color)
{
	switch(color)
	{
	case AO_LED_COLOR_RED: return "ROJO";
	case AO_LED_COLOR_BLUE: return "AZUL";
	case AO_LED_COLOR_GREEN: return "VERDE";
	default: return "INVALIDO";
	}
	return "INVALIDO";
}

static void task_(void *argument)
{
  (void)argument;
  while (true)
  {
    ao_led_message_t msg;
    if (pdPASS == xQueueReceive(hqueue, &msg, portMAX_DELAY))
    {
      switch (msg.action) {
        case AO_LED_MESSAGE_ON:
          vTaskDelay(pdMS_TO_TICKS(5)); // Small delay to avoid log corruption
          LOGGER_INFO("LED %s ENCENDIDO", ledColorToStr(msg.color));
          if (msg.callback) {
              msg.callback(&msg);
          }
          break;

        case AO_LED_MESSAGE_OFF:
          vTaskDelay(pdMS_TO_TICKS(5)); // Small delay to avoid log corruption
          LOGGER_INFO("LED %s APAGADO", ledColorToStr(msg.color));
          if (msg.callback) {
              msg.callback(&msg);
          }
          break;

        default:
          break;
      }
    }
  }
}

BaseType_t ao_led_create_task(){
  return xTaskCreate(task_, "task_ao_led", 128, NULL, tskIDLE_PRIORITY, NULL);
}

/********************** external functions definition ************************/

bool ao_led_send(ao_led_message_t* msg)
{
    if (msg == NULL) {
        LOGGER_INFO("LED message is NULL");
        return false;
    }

    return (xQueueSend(hqueue, msg, pdMS_TO_TICKS(100)) == pdPASS);
}

void ao_led_init(void)
{
    // Crear la cola de mensajes
    hqueue = xQueueCreate(QUEUE_LENGTH_, QUEUE_ITEM_SIZE_);
    configASSERT(hqueue != NULL);
    
    // Crear la tarea del LED
    BaseType_t status = xTaskCreate(task_, "task_led", 256, NULL, tskIDLE_PRIORITY + 1, NULL);  // Increased stack and priority
    configASSERT(status == pdPASS);
    LOGGER_INFO("AO LED initialized");
}

/********************** end of file ******************************************/
