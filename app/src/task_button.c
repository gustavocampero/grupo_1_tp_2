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

#include "main.h"
#include "cmsis_os.h"
#include "board.h"
#include "logger.h"
#include "dwt.h"

#include "task_ui.h"

/********************** macros and definitions *******************************/

#define TASK_PERIOD_MS_           (50)    // Período de polling del botón

// Timeouts para detección de tipos de pulsación
#define BUTTON_PULSE_TIMEOUT_     (200)   // Pulsación rápida: 0-200ms
#define BUTTON_SHORT_TIMEOUT_     (1000)  // Pulsación corta: 200-1000ms
#define BUTTON_LONG_TIMEOUT_      (2000)  // Pulsación larga: >1000ms

/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/

/********************** external data definition *****************************/

/********************** internal functions definition ************************/

typedef enum
{
  BUTTON_TYPE_NONE,
  BUTTON_TYPE_PULSE,
  BUTTON_TYPE_SHORT,
  BUTTON_TYPE_LONG,
  BUTTON_TYPE__N,
} button_type_t;

static struct
{
    uint32_t counter;
} button;

static void button_init_(void)
{
  button.counter = 0;
}

static button_type_t button_process_state_(bool value)
{
  button_type_t ret = BUTTON_TYPE_NONE;
  if(value)
  {
    button.counter += TASK_PERIOD_MS_;
  }
  else
  {
    if(BUTTON_LONG_TIMEOUT_ <= button.counter)
    {
      ret = BUTTON_TYPE_LONG;
    }
    else if(BUTTON_SHORT_TIMEOUT_ <= button.counter)
    {
      ret = BUTTON_TYPE_SHORT;
    }
    else if(BUTTON_PULSE_TIMEOUT_ <= button.counter)
    {
      ret = BUTTON_TYPE_PULSE;
    }
    button.counter = 0;
  }
  return ret;
}

/********************** external functions definition ************************/

void task_button(void* argument)
{
  // Validar hardware del botón
  if (HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN) == GPIO_PIN_RESET) {
    LOGGER_ERROR("Button: Error inicializando hardware");
    return;
  }

  button_init_();
  LOGGER_INFO("Button: Tarea iniciada");

  while(true)
  {
    GPIO_PinState button_state = HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN);
    button_type_t button_type = button_process_state_(!button_state);

    // Procesar evento si hay uno
    if (button_type != BUTTON_TYPE_NONE) {
      msg_event_t event;
      // Evaluar estado del botón

      switch (button_type) {
        case BUTTON_TYPE_PULSE:
          LOGGER_DEBUG("Button: Pulso detectado");
          event = MSG_EVENT_BUTTON_PULSE;
          break;
        case BUTTON_TYPE_SHORT:
          LOGGER_DEBUG("Button: Pulsación corta detectada");
          event = MSG_EVENT_BUTTON_SHORT;
          break;
        case BUTTON_TYPE_LONG:
          LOGGER_DEBUG("Button: Pulsación larga detectada");
          event = MSG_EVENT_BUTTON_LONG;
          break;
        default:
          LOGGER_ERROR("Button: Tipo de pulsación inválido");
          continue;
      }

      // Enviar evento a UI
      if (!ao_ui_send_event(event)) {
        LOGGER_ERROR("Button: Error enviando evento a UI");
      }
    }

    vTaskDelay((TickType_t)(TASK_PERIOD_MS_ / portTICK_PERIOD_MS));
  }
}

/********************** end of file ******************************************/
