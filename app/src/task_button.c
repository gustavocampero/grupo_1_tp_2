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
#include "task_button.h"
#include "delay.h"

/********************** macros and definitions *******************************/

#define TASK_PERIOD_MS_           (50)

#define BUTTON_PERIOD_MS_         (TASK_PERIOD_MS_)
#define BUTTON_PULSE_TIMEOUT_     (200)
#define BUTTON_SHORT_TIMEOUT_     (1000)
#define BUTTON_LONG_TIMEOUT_      (2000)

/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/

/********************** external data definition *****************************/

extern SemaphoreHandle_t hsem_button;

/********************** internal functions definition ************************/

static struct {
    uint32_t counter;
    delay_t debounce_delay;
    bool last_state;
} button;

static void button_init_(void)
{
    button.counter = 0;
    button.last_state = false;
    delayInit(&button.debounce_delay, 20); // 20ms de debounce
}

static button_state_t button_process_state_(bool value)
{
    button_state_t ret = BUTTON_STATE_NONE;
    
    // Debouncing
    if (value != button.last_state) {
        delayInit(&button.debounce_delay, 20);
        button.last_state = value;
    }

    if (delayRead(&button.debounce_delay)) {
        if (value) {
            button.counter += BUTTON_PERIOD_MS_;
        } else {
            if (BUTTON_LONG_TIMEOUT_ <= button.counter)
            {
                ret = BUTTON_STATE_LONG;
            }
            else if (BUTTON_SHORT_TIMEOUT_ <= button.counter)
            {
                ret = BUTTON_STATE_SHORT;
            }
            else if (BUTTON_PULSE_TIMEOUT_ <= button.counter)
            {
                ret = BUTTON_STATE_PULSE;
            }
            button.counter = 0;
        }
    }
    return ret;
}

/********************** external functions definition ************************/

void task_button(void* argument)
{
    button_init_();
    
    while(true)
    {
        GPIO_PinState button_state;
        button_state = HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN);

        button_state_t state = button_process_state_(!button_state);
        
        if (state != BUTTON_STATE_NONE) {
            // Crear evento del botón
            button_event_t event = {
                .state = state,
                .timestamp = HAL_GetTick()
            };
            
            // Determinar estado y prioridad para logging
            const char* state_str;
            // Convertir el estado del botón a evento de mensaje
            msg_event_t msg_event;
            switch (state) {
                case BUTTON_STATE_PULSE:
                    msg_event = MSG_EVENT_BUTTON_PULSE;
                    state_str = "pulse";
                    break;
                case BUTTON_STATE_SHORT:
                    msg_event = MSG_EVENT_BUTTON_SHORT;
                    state_str = "short";
                    break;
                case BUTTON_STATE_LONG:
                    msg_event = MSG_EVENT_BUTTON_LONG;
                    state_str = "long";
                    break;
                default:
                    state_str = "unknown";
                    continue; // Saltar este evento
            }
            
            // Loggear el evento del botón
            LOGGER_INFO("Button %s press detected at tick %lu", 
                     state_str, event.timestamp);
            
            // Enviar el evento al UI
            ao_ui_send_event(msg_event);
        }

    vTaskDelay((TickType_t)(TASK_PERIOD_MS_ / portTICK_PERIOD_MS));
  }
}

/********************** end of file ******************************************/
