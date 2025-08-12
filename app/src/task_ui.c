/*
 * Copyright (c) 2024 Sebastian Bedin <sebabedin@gmail.com>.
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
 *
 * @file   : ao_led.c
 * @date   : Feb 17, 2023
 * @author : Sebastian Bedin <sebabedin@gmail.com>
 * @version	v1.0.0
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
#include "task_led.h"
#include "memory_pool.h"
#include "priority_queue.h"

/********************** macros and definitions *******************************/

#define QUEUE_SIZE               (10)    // Tamaño de la cola de prioridad
#define MEMORY_POOL_NBLOCKS     (10)
#define MEMORY_POOL_BLOCK_SIZE  (sizeof(ao_led_message_t))
#define LED_ON_TIME_MS         (5000)   // 5 segundos

/********************** internal data declaration ****************************/

// Estructura para eventos UI con prioridad
typedef struct {
    msg_event_t type;          // Tipo de evento del botón
    uint32_t timestamp;        // Para orden FIFO dentro de misma prioridad
    priority_level_t priority; // Prioridad del evento
} ui_event_t;

// Estructura principal del AO
typedef struct {
    priority_queue_t priority_queue;    // Cola de prioridad única
    uint8_t high_buffer[QUEUE_SIZE * sizeof(ui_event_t)];
    uint8_t medium_buffer[QUEUE_SIZE * sizeof(ui_event_t)];
    uint8_t low_buffer[QUEUE_SIZE * sizeof(ui_event_t)];
    SemaphoreHandle_t led_mutex;       // Mutex para proteger acceso a LEDs
    bool led_active;                   // Indica si hay LED activo
    ao_led_color active_led;           // LED actualmente activo
    TimerHandle_t led_timer;           // Timer para apagar LED
} ao_ui_handle_t;

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/

static ao_ui_handle_t hao_;
static memory_pool_t memory_pool_;
static uint8_t memory_pool_memory_[MEMORY_POOL_SIZE(MEMORY_POOL_NBLOCKS, MEMORY_POOL_BLOCK_SIZE)];
memory_pool_t* const hmp = &memory_pool_;

/********************** internal functions definition ************************/

static void callback_(void* ptr)
{
    memory_pool_block_put(hmp, (void*)ptr);
}

// Estructura de control del LED
typedef struct {
    ao_led_color current_color;
    bool is_processing;
    TimerHandle_t timer;
    SemaphoreHandle_t mutex;
} led_control_t;

static led_control_t led_control = {
    .current_color = AO_LED_COLOR_RED,
    .is_processing = false,
    .timer = NULL,
    .mutex = NULL
};

static void led_timer_callback(TimerHandle_t timer) {
    if (xSemaphoreTake(led_control.mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        // Apagar el LED actual
        ao_led_message_t* led_msg = (ao_led_message_t*)memory_pool_block_get(hmp);
        if (led_msg != NULL) {
            led_msg->callback = callback_;
            led_msg->action = AO_LED_MESSAGE_OFF;
            led_msg->color = led_control.current_color;
            led_msg->value = 0;
            
            if (ao_led_send(led_msg) == false) {
                memory_pool_block_put(hmp, (void*)led_msg);
            }
        }
        led_control.is_processing = false;
        xSemaphoreGive(led_control.mutex);
    }
}

static bool sendmsg(ao_led_color color, ao_led_action_t action, int value)
{
    bool success = false;
    
    if (xSemaphoreTake(led_control.mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        // Solo proceder si no hay un LED activo o es una acción de apagado
        if (!led_control.is_processing || action == AO_LED_MESSAGE_OFF) {
            ao_led_message_t* led_msg = (ao_led_message_t*)memory_pool_block_get(hmp);
            if(led_msg != NULL) {
                led_msg->callback = callback_;
                led_msg->action = action;
                led_msg->value = value;
                led_msg->color = color;
                
                if(ao_led_send(led_msg)) {
                    success = true;
                    if (action == AO_LED_MESSAGE_ON) {
                        led_control.current_color = color;
                        led_control.is_processing = true;
                        xTimerStart(led_control.timer, pdMS_TO_TICKS(100));
                    }
                } else {
                    memory_pool_block_put(hmp, (void*)led_msg);
                }
            }
        }
        xSemaphoreGive(led_control.mutex);
    }
    return success;
}

// Timer callback moved to top of file

static void task_(void *argument)
{
    // Inicializar memory pool para mensajes LED
    memory_pool_init(hmp, memory_pool_memory_, MEMORY_POOL_NBLOCKS, MEMORY_POOL_BLOCK_SIZE);

    // Inicializar cola de prioridad
    priority_queue_init(&hao_.priority_queue, 
                       hao_.high_buffer,
                       hao_.medium_buffer,
                       hao_.low_buffer,
                       QUEUE_SIZE * sizeof(ui_event_t));

    // Crear mutex para protección del estado del LED
    led_control.mutex = xSemaphoreCreateMutex();
    assert(led_control.mutex != NULL);

    // Crear timer para apagado automático del LED
    led_control.timer = xTimerCreate("LED Timer", 
                                   pdMS_TO_TICKS(LED_ON_TIME_MS),
                                   pdFALSE,  // No auto-reload
                                 NULL,
                                 led_timer_callback);
    assert(hao_.led_timer != NULL);

    // Inicializar estado
    hao_.led_active = false;

    // Apagar todos los LEDs al inicio
    ao_led_message_t* led_msg = (ao_led_message_t*)memory_pool_block_get(hmp);
    if (led_msg != NULL) {
        led_msg->callback = callback_;
        led_msg->action = AO_LED_MESSAGE_OFF;
        led_msg->value = 0;
        
        // Apagar cada LED
        for (int i = 0; i < 3; i++) {
            led_msg->color = (ao_led_color)i;
            if (ao_led_send(led_msg) == false) {
                memory_pool_block_put(hmp, (void*)led_msg);
                break;
            }
        }
        memory_pool_block_put(hmp, (void*)led_msg);
    }

    while (true)
    {
        priority_queue_msg_t event;
        
        // Si no hay LED activo, procesar siguiente evento
        if (!led_control.is_processing) {
            if (priority_queue_receive(&hao_.priority_queue, &event)) {
                ao_led_color led_color;
                const char* priority_str;
                
                // Determinar color según tipo de evento
                switch (event.type) {
                    case MSG_EVENT_BUTTON_PULSE:
                        led_color = AO_LED_COLOR_RED;
                        priority_str = "high";
                        break;
                    case MSG_EVENT_BUTTON_SHORT:
                        led_color = AO_LED_COLOR_GREEN;
                        priority_str = "medium";
                        break;
                    case MSG_EVENT_BUTTON_LONG:
                        led_color = AO_LED_COLOR_BLUE;
                        priority_str = "low";
                        break;
                    default:
                        continue;
                }
                
                // Activar el LED correspondiente y registrar evento
                LOGGER_INFO("Processing %s priority event - timestamp: %lu", 
                          priority_str, event.timestamp);
                
                if (sendmsg(led_color, AO_LED_MESSAGE_ON, 0)) {
                    LOGGER_INFO("LED %s activated successfully", priority_str);
                } else {
                    LOGGER_INFO("Failed to activate LED %s", priority_str);
                }
            }
        }
        
        // Dar tiempo para que otras tareas se ejecuten
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/********************** external functions definition ************************/

bool ao_ui_send_event(msg_event_t msg)
{
    ui_event_t event;
    event.timestamp = HAL_GetTick();
    event.type = msg;
    
    // Asignar prioridad según el tipo de evento
    switch(msg) {
        case MSG_EVENT_BUTTON_PULSE:
            event.priority = PRIORITY_HIGH;
            break;
        case MSG_EVENT_BUTTON_SHORT:
            event.priority = PRIORITY_MEDIUM;
            break;
        case MSG_EVENT_BUTTON_LONG:
            event.priority = PRIORITY_LOW;
            break;
        default:
            return false;
    }
    
    LOGGER_INFO("Sending event type=%d with priority=%d at timestamp=%lu", 
               event.type, event.priority, event.timestamp);
    
    return priority_queue_send(&hao_.priority_queue, (priority_queue_msg_t*)&event);
}

void ao_ui_init(void)
{    
    BaseType_t status;
    status = xTaskCreate(task_, "task_ao_ui", 256, NULL, tskIDLE_PRIORITY, NULL);
    assert(pdPASS == status);
}

/********************** end of file ******************************************/
