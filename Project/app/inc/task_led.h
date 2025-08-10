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

#ifndef TASK_LED_H_
#define TASK_LED_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusions *******************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "main.h"
#include "cmsis_os.h"
/********************** macros ***********************************************/

/********************** typedef **********************************************/

typedef enum
{
  AO_LED_MESSAGE_ON,
  AO_LED_MESSAGE_OFF,
  AO_LED_MESSAGE_BLINK,
  AO_LED_MESSAGE__N,
} ao_led_action_t;

typedef enum
{
  AO_LED_STATUS_OK,               // La operación se completó exitosamente
  AO_LED_STATUS_INVALID_PARAMS,   // Los parámetros del mensaje son inválidos
  AO_LED_STATUS_INVALID_ACTION,   // La acción solicitada es inválida/desconocida
  AO_LED_STATUS_HARDWARE_ERROR,   // Error al manipular el hardware del LED
} ao_led_status_t;

typedef void (*ao_led_cb_t)(int id, ao_led_status_t status);

typedef struct
{
    int id;
    ao_led_action_t action;
    int value;
} ao_led_data_t;

typedef struct
{
    ao_led_data_t data;
    ao_led_cb_t callback;
    ao_led_status_t status;       // Resultado de la operación
} ao_led_message_t;

typedef enum
{
  AO_LED_COLOR_RED,
  AO_LED_COLOR_GREEN,
  AO_LED_COLOR_BLUE,
} ao_led_color;

typedef struct
{
    ao_led_color color;
    QueueHandle_t hqueue;           // Cola para mensajes
    bool is_active;                 // Flag para saber si la tarea está activa
    TaskHandle_t task_handle;       // Handle de la tarea
} ao_led_handle_t;

/********************** external data declaration ****************************/

/********************** external functions declaration ***********************/

// Inicializa el handle del LED (sin crear la tarea)
void ao_led_init(ao_led_handle_t* hao, ao_led_color color);

// Crea la tarea LED si no está activa
BaseType_t ao_led_start_task(ao_led_handle_t* hao);

// Envía un mensaje al LED
bool ao_led_send(ao_led_handle_t* hao, ao_led_message_t* msg);

// Función auxiliar para convertir color a string
const char* ledColorToStr(ao_led_color color);

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* TASK_LED_H_ */
/********************** end of file ******************************************/


