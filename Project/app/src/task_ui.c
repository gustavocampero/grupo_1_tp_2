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
#include "FreeRTOS.h" // Para pvPortMalloc y vPortFree

#include "main.h"
#include "cmsis_os.h"
#include "board.h"
#include "logger.h"
#include "dwt.h"

#include "task_ui.h"
#include "task_led.h"
#include "memory_pool.h"
#include "logger.h"
#include "board.h"
#include "linked_list.h" // Para la lista enlazada de mensajes activos

/********************** macros and definitions *******************************/

#define QUEUE_LENGTH_            (5)  // Permitir múltiples eventos en cola
#define QUEUE_ITEM_SIZE_         (sizeof(msg_event_t))
#define UI_MSG_POOL_BLOCKS       (10)    // Cantidad de mensajes en la pool
#define UI_MSG_POOL_SIZE         (MEMORY_POOL_SIZE(UI_MSG_POOL_BLOCKS, sizeof(ao_led_message_t)))

/********************** internal data declaration ****************************/

typedef struct
{
    QueueHandle_t hqueue;          // Cola para eventos del botón
    memory_pool_t msg_pool;        // Pool de mensajes LED
    uint8_t pool_memory[UI_MSG_POOL_SIZE];
} ao_ui_handle_t;

// Reemplazar el array por una lista enlazada de mensajes activos
typedef struct {
    int id;
    ao_led_message_t* msg;
} active_message_entry_t;

static linked_list_t active_messages_list;

// LED handles - owned by UI since it manages their lifecycle
static ao_led_handle_t led_red;
static ao_led_handle_t led_green;
static ao_led_handle_t led_blue;

/********************** internal functions declaration ***********************/

static ao_led_message_t* create_led_message(int id, ao_led_action_t action, int value);
static void free_led_message(ao_led_message_t* msg);
static void send_led_on(ao_led_handle_t* hled);
static void turn_off_all_leds(void);
static void led_callback(int id, ao_led_status_t status);

/********************** internal data definition *****************************/

static ao_ui_handle_t hao_;

/********************** internal functions definition ************************/

static ao_led_message_t* create_led_message(int id, ao_led_action_t action, int value)
{
    // Obtener un bloque de memoria de la pool
    ao_led_message_t* msg = (ao_led_message_t*)memory_pool_block_get(&hao_.msg_pool);
    if (msg == NULL) {
        //LOGGER_ERROR("UI: No hay memoria disponible para mensaje LED");
        return NULL;
    }

    // Inicializar el mensaje
    msg->data.id = id;
    msg->data.action = action;
    msg->data.value = value;
    msg->callback = led_callback;
    msg->status = AO_LED_STATUS_OK;

    // Crear entry dinámico y agregarlo a la lista
    active_message_entry_t* entry = (active_message_entry_t*)pvPortMalloc(sizeof(active_message_entry_t));
    if (entry == NULL) {
        //LOGGER_ERROR("UI: No hay memoria para entry de mensaje activo");
        memory_pool_block_put(&hao_.msg_pool, msg);
        return NULL;
    }
    entry->id = id;
    entry->msg = msg;
    linked_list_node_t* node = (linked_list_node_t*)pvPortMalloc(sizeof(linked_list_node_t));
    if (node == NULL) {
        //LOGGER_ERROR("UI: No hay memoria para nodo de lista de mensajes activos");
        vPortFree(entry);
        memory_pool_block_put(&hao_.msg_pool, msg);
        return NULL;
    }
    linked_list_node_init(node, entry);
    linked_list_node_add(&active_messages_list, node);

    return msg;
}

static void free_led_message(ao_led_message_t* msg)
{
    if (msg != NULL) {
        memory_pool_block_put(&hao_.msg_pool, msg);
    }
}

static void send_led_on(ao_led_handle_t* hled)
{
    // Crear tarea LED si no está activa
    if (!hled->is_active) {
        if (pdPASS != ao_led_start_task(hled)) {
            //LOGGER_ERROR("UI: Error al crear tarea LED %s", ledColorToStr(hled->color));
            return;
        }
    }

    static int msg_id = 0;
    ao_led_message_t* msg = create_led_message(++msg_id, AO_LED_MESSAGE_ON, 0);
    if (msg != NULL) {
        if (!ao_led_send(hled, msg)) {
            //LOGGER_ERROR("UI: Error al enviar mensaje a LED");
            free_led_message(msg);  // Solo liberamos si falló el envío
        }
        // Nota: Si el envío es exitoso, la memoria se liberará en el callback
        // cuando la tarea LED termine de procesar el mensaje
    }
}

static void turn_off_all_leds(void)
{
    // Apagar todos los LEDs antes de encender uno nuevo
    static int msg_id = 100;  // Usar IDs diferentes para evitar conflictos
    
    ao_led_message_t* msg_red = create_led_message(++msg_id, AO_LED_MESSAGE_OFF, 0);
    ao_led_message_t* msg_green = create_led_message(++msg_id, AO_LED_MESSAGE_OFF, 0);
    ao_led_message_t* msg_blue = create_led_message(++msg_id, AO_LED_MESSAGE_OFF, 0);

    if (msg_red) {
        ao_led_send(&led_red, msg_red);
    }
    if (msg_green) {
        ao_led_send(&led_green, msg_green);
    }
    if (msg_blue) {
        ao_led_send(&led_blue, msg_blue);
    }
}

static void led_callback(int id, ao_led_status_t status)
{
    if (status != AO_LED_STATUS_OK) {
        //LOGGER_ERROR("UI: LED mensaje %d completado con error: %d", id, status);
    } else {
        //LOGGER_DEBUG("UI: LED mensaje %d completado OK", id);
    }
    // Buscar y eliminar el mensaje de la lista por ID
    linked_list_node_t* node = linked_list_node_remove_by_id(&active_messages_list, id);
    if (node != NULL) {
        active_message_entry_t* entry = (active_message_entry_t*)node->pdata;
        if (entry) {
            free_led_message(entry->msg); // Devuelve el mensaje a la memory pool
            vPortFree(entry); // Libera el entry
        }
        vPortFree(node); // Libera el nodo
        //LOGGER_DEBUG("UI: Memoria del mensaje %d liberada", id);
    }
}

static void task_(void *argument)
{
  // No necesitamos inicializar los LEDs aquí, se crearán bajo demanda

  while (true)
  {

    msg_event_t event_msg;

    if (pdPASS == xQueueReceive(hao_.hqueue, &event_msg, portMAX_DELAY))
    {
      switch (event_msg)
      {
        case MSG_EVENT_BUTTON_PULSE:
          //LOGGER_INFO("Estado Rojo - LED Rojo encendido");
          turn_off_all_leds();  // Apagar todos antes de encender el rojo
          send_led_on(&led_red);
          break;
        case MSG_EVENT_BUTTON_SHORT:
          //LOGGER_INFO("Estado Verde - LED Verde encendido");
          turn_off_all_leds();  // Apagar todos antes de encender el verde
          send_led_on(&led_green);
          break;
        case MSG_EVENT_BUTTON_LONG:
          //LOGGER_INFO("Estado Azul - LED Azul encendido");
          turn_off_all_leds();  // Apagar todos antes de encender el azul
          send_led_on(&led_blue);
          break;
        default:
          break;
      }
    }
  }
}

/********************** external functions PULSEdefinition ************************/

bool ao_ui_send_event(msg_event_t msg)
{
  return (pdPASS == xQueueSend(hao_.hqueue, (void*)&msg, 0));
}

void ao_ui_init(void)
{
  // Inicializar cola para eventos del botón
  hao_.hqueue = xQueueCreate(QUEUE_LENGTH_, QUEUE_ITEM_SIZE_);
  if (hao_.hqueue == NULL)
  {
    //LOGGER_ERROR("Failed to create UI event queue");
    while(1);
  }

  // Inicializar memory pool para mensajes LED
  memory_pool_init(&hao_.msg_pool, 
                  hao_.pool_memory,
                  UI_MSG_POOL_BLOCKS, 
                  sizeof(ao_led_message_t));
  
  // Initialize LEDs
  //LOGGER_INFO("Initializing LEDs...");

  // Inicializar handles de LEDs
  ao_led_init(&led_red, AO_LED_COLOR_RED);
  ao_led_init(&led_green, AO_LED_COLOR_GREEN);
  ao_led_init(&led_blue, AO_LED_COLOR_BLUE);

  // Crear tareas LED
  ao_led_start_task(&led_red);
  ao_led_start_task(&led_green);
  ao_led_start_task(&led_blue);

  // Crear tarea UI
  BaseType_t status;
  status = xTaskCreate(task_, "task_ao_ui", 128, NULL, tskIDLE_PRIORITY, NULL);
  while (pdPASS != status)
  {
    // Error: No se pudo crear la tarea UI
    //LOGGER_ERROR("UI: Error creando tarea UI");
  }

  // Inicializar la lista enlazada de mensajes activos
  linked_list_init(&active_messages_list);

  // Apagar todos los LEDs usando el sistema de mensajes normal
  ao_led_message_t* msg_red = create_led_message(1, AO_LED_MESSAGE_OFF, 0);
  ao_led_message_t* msg_green = create_led_message(2, AO_LED_MESSAGE_OFF, 0);
  ao_led_message_t* msg_blue = create_led_message(3, AO_LED_MESSAGE_OFF, 0);

  if (msg_red) {
    ao_led_send(&led_red, msg_red);
  }
  if (msg_green) {
    ao_led_send(&led_green, msg_green);
  }
  if (msg_blue) {
    ao_led_send(&led_blue, msg_blue);
  }
}

/********************** end of file ******************************************/
