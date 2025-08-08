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

#define LED_TASK_PERIOD_MS_       (1000)
#define TASK_INACTIVITY_TIMEOUT_MS_ (500)  // Timeout para auto-eliminarse

#define QUEUE_LENGTH_            (10)
#define QUEUE_ITEM_SIZE_         (sizeof(ao_led_message_t))

/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/

static GPIO_TypeDef* led_port_[] = {LED_RED_PORT, LED_GREEN_PORT,  LED_BLUE_PORT};
static uint16_t led_pin_[] = {LED_RED_PIN,  LED_GREEN_PIN, LED_BLUE_PIN };

/********************** external data definition *****************************/

/********************** internal functions definition ************************/

const char* ledColorToStr(ao_led_color color)
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
  ao_led_handle_t* hao = (ao_led_handle_t*)argument;
  if (NULL == hao) {
    LOGGER_ERROR("LED task iniciada con handle nulo");
    return;
  }

  LOGGER_INFO("Iniciando tarea LED %s", ledColorToStr(hao->color));
  
  while (true)
  {
    ao_led_message_t* msg;
    if (pdPASS == xQueueReceive(hao->hqueue, (void*)&msg, pdMS_TO_TICKS(TASK_INACTIVITY_TIMEOUT_MS_)))
    {
      // Inicializamos asumiendo éxito
      msg->status = AO_LED_STATUS_OK;

      // Validación básica del mensaje
      if (NULL == msg || NULL == msg->callback) {
        LOGGER_ERROR("LED %s: Mensaje invalido recibido", ledColorToStr(hao->color));
        if (msg != NULL && msg->callback != NULL) {
            msg->callback(msg->data.id, AO_LED_STATUS_INVALID_PARAMS);
        }
        continue;
      }

      LOGGER_DEBUG("LED %s: Mensaje recibido [ID:%d, Action:%d]", 
                  ledColorToStr(hao->color), msg->data.id, msg->data.action);

      switch (msg->data.action) {
        case AO_LED_MESSAGE_ON:
          HAL_GPIO_WritePin(led_port_[hao->color], led_pin_[hao->color], GPIO_PIN_SET);
          LOGGER_INFO("LED %s ENCENDIDO", ledColorToStr(hao->color));
          break;

        case AO_LED_MESSAGE_OFF:
          HAL_GPIO_WritePin(led_port_[hao->color], led_pin_[hao->color], GPIO_PIN_RESET);
          LOGGER_INFO("LED %s APAGADO", ledColorToStr(hao->color));
          break;

        case AO_LED_MESSAGE_BLINK:
          if (msg->data.value <= 0) {
            msg->status = AO_LED_STATUS_INVALID_PARAMS;
            LOGGER_WARNING("LED %s: Tiempo de parpadeo invalido (%d)", ledColorToStr(hao->color), msg->data.value);
            break;
          }

          LOGGER_INFO("LED %s: Iniciando parpadeo por %dms", ledColorToStr(hao->color), msg->data.value);
          HAL_GPIO_WritePin(led_port_[hao->color], led_pin_[hao->color], GPIO_PIN_SET);
          vTaskDelay((TickType_t)((msg->data.value) / portTICK_PERIOD_MS));
          HAL_GPIO_WritePin(led_port_[hao->color], led_pin_[hao->color], GPIO_PIN_RESET);
          break;

        default:
          msg->status = AO_LED_STATUS_INVALID_ACTION;
          LOGGER_ERROR("LED %s: Accion desconocida recibida (%d)", ledColorToStr(hao->color), msg->data.action);
          break;
      }
      
      // Notificamos el resultado
      msg->callback(msg->data.id, msg->status);
    }
    else {
      // No hay mensajes después del timeout, auto-destruir
      LOGGER_INFO("LED %s: Sin mensajes por %dms, auto-destruyendo", 
                 ledColorToStr(hao->color), TASK_INACTIVITY_TIMEOUT_MS_);
      hao->is_active = false;
      hao->task_handle = NULL;
      vTaskDelete(NULL);
      break;
    }
  }
}

/********************** external functions definition ************************/

bool ao_led_send(ao_led_handle_t* hao, ao_led_message_t* msg)
{
  if (NULL == hao || NULL == msg || NULL == msg->callback) {
    LOGGER_ERROR("ao_led_send: Parametros invalidos");
    return false;
  }

  if (msg->data.action >= AO_LED_MESSAGE__N) {
    LOGGER_ERROR("ao_led_send: Accion invalida (%d)", msg->data.action);
    return false;
  }

  if (msg->data.action == AO_LED_MESSAGE_BLINK && msg->data.value <= 0) {
    LOGGER_ERROR("ao_led_send: Tiempo de parpadeo invalido (%d)", msg->data.value);
    return false;
  }

  // Inicializamos el status
  msg->status = AO_LED_STATUS_OK;

  return (pdPASS == xQueueSend(hao->hqueue, (void*)msg, 0));
}

void ao_led_init(ao_led_handle_t* hao, ao_led_color color)
{
  hao->color = color;
  hao->is_active = false;
  hao->task_handle = NULL;

  hao->hqueue = xQueueCreate(QUEUE_LENGTH_, QUEUE_ITEM_SIZE_);
  while(NULL == hao->hqueue)
  {
    // Error: No se pudo crear la cola del LED
    LOGGER_ERROR("LED %s: Error creando cola", ledColorToStr(hao->color));
  }
}

BaseType_t ao_led_start_task(ao_led_handle_t* hao)
{
  if(hao->is_active) {
    return pdPASS; // Ya está activa
  }

  BaseType_t status = xTaskCreate(task_, "task_ao_led", 128, 
                                (void*)hao, tskIDLE_PRIORITY, &hao->task_handle);
  if(pdPASS == status) {
    hao->is_active = true;
    LOGGER_INFO("LED %s: Tarea creada", ledColorToStr(hao->color));
  }
  return status;
}

/********************** end of file ******************************************/
