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

static GPIO_TypeDef* led_port_[] = {LED_RED_PORT, LED_GREEN_PORT,  LED_BLUE_PORT};
static uint16_t led_pin_[] = {LED_RED_PIN,  LED_GREEN_PIN, LED_BLUE_PIN };
static bool task_led_running = false; // sólo 1 hilo para manejar los leds
static QueueHandle_t hqueue;
static SemaphoreHandle_t led_mutex = NULL;

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
    ao_led_message_t* msg;
    if (xSemaphoreTake(led_mutex, portMAX_DELAY) == pdTRUE) {} //  tomo el mutex
    if (pdPASS == xQueueReceive(hqueue, (void*)&msg, 0))
    {
      xSemaphoreGive(led_mutex);
      switch (msg->action) {
        case AO_LED_MESSAGE_ON:
          HAL_GPIO_WritePin(led_port_[msg->color], led_pin_[msg->color], GPIO_PIN_SET);
          LOGGER_INFO("				LED %s ENCENDIDO", ledColorToStr(msg->color));
          msg->callback((void*)msg);
          break;

        case AO_LED_MESSAGE_OFF:
          HAL_GPIO_WritePin(led_port_[msg->color], led_pin_[msg->color], GPIO_PIN_RESET);
          LOGGER_INFO("				LED %s APAGADO", ledColorToStr(msg->color));
          msg->callback((void*)msg);
          break;

        case AO_LED_MESSAGE_BLINK:
          HAL_GPIO_WritePin(led_port_[msg->color], led_pin_[msg->color], GPIO_PIN_SET);
          LOGGER_INFO("				LED %s ENCENDIDO", ledColorToStr(msg->color));
          vTaskDelay((TickType_t)((msg->value) / portTICK_PERIOD_MS));
          HAL_GPIO_WritePin(led_port_[msg->color], led_pin_[msg->color], GPIO_PIN_RESET);
          LOGGER_INFO("				LED %s APAGADO", ledColorToStr(msg->color));
          msg->callback((void*)msg);
          break;

        default:
          break;
      }
    }else{
    	task_led_running = false;
    	xSemaphoreGive(led_mutex); // Libero el mutex antes de eliminar la tarea
    	vTaskDelete(NULL);
    }
    vTaskDelay((TickType_t)(50 / portTICK_PERIOD_MS)); // Si no, la button_task se bloquea hasta que se termine de procesar la accion
  }
}

BaseType_t ao_led_create_task(){
  return xTaskCreate(task_, "task_ao_led", 128, NULL, tskIDLE_PRIORITY, NULL);
}

/********************** external functions definition ************************/

bool ao_led_send(ao_led_message_t* msg)
{
	if (xSemaphoreTake(led_mutex, portMAX_DELAY) == pdTRUE) {
		if(!task_led_running) {
			if(ao_led_create_task() != pdPASS) {
				LOGGER_INFO("ERROR CREANDO TAREA LEDS!");
			}else {
				// LOGGER_INFO("Tarea Leds creada");
				task_led_running = true;
			}
		}
		xSemaphoreGive(led_mutex);
	}

	return (xQueueSend(hqueue,(void*)&msg,0) == pdPASS);
}

void ao_led_init()
{
  hqueue = xQueueCreate(QUEUE_LENGTH_, QUEUE_ITEM_SIZE_);
  if(NULL == hqueue){
	  LOGGER_INFO("ERROR: ao_led_init xQueueCreate");
	  while (true){/*ERROR*/}
  }

  led_mutex = xSemaphoreCreateMutex();
  if (NULL == led_mutex) {
	  LOGGER_INFO("ERROR: ao_led_init xSemaphoreCreateMutex");
	  while (true) {};
  }

}

/********************** end of file ******************************************/
