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

/********************** macros and definitions *******************************/

#define QUEUE_LENGTH_            (1)
#define QUEUE_ITEM_SIZE_         (sizeof(msg_event_t))

#define MEMORY_POOL_NBLOCKS       (10)
#define MEMORY_POOL_BLOCK_SIZE    (sizeof(ao_led_message_t))

/********************** internal data declaration ****************************/

typedef struct
{
    QueueHandle_t hqueue;
} ao_ui_handle_t;

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/

static ao_ui_handle_t hao_;
static int msg_wip_ = 0;
static memory_pool_t memory_pool_;
static uint8_t memory_pool_memory_[MEMORY_POOL_SIZE(MEMORY_POOL_NBLOCKS, MEMORY_POOL_BLOCK_SIZE)];

/********************** external data definition *****************************/



memory_pool_t* const hmp = &memory_pool_;

/********************** internal functions definition ************************/

static void callback_(void* ptr)
{
	memory_pool_block_put(hmp, (void*)ptr);
    LOGGER_INFO("Memoria liberada desde button");
    LOGGER_INFO("Mensajes en proceso: %d", --msg_wip_);
}

static void sendmsg(ao_led_color color ,ao_led_action_t action, int value)
{
	static int id = 0;
	ao_led_message_t* led_msg = (ao_led_message_t*)memory_pool_block_get(hmp);
	if(NULL != led_msg)
	{
	  led_msg->callback = callback_;
	  led_msg->id = id++;
	  led_msg->action = action;
	  led_msg->value = value;
	  led_msg->color = color;
	  vTaskDelay((TickType_t)(50 / portTICK_PERIOD_MS)); // Si no, la button_task se bloquea hasta que se termine de procesar la accion
	  ao_led_send(led_msg);
	  msg_wip_++;
	}
}

static void task_(void *argument)
{
  memory_pool_init(hmp, memory_pool_memory_, MEMORY_POOL_NBLOCKS, MEMORY_POOL_BLOCK_SIZE);

  sendmsg(AO_LED_COLOR_RED   , AO_LED_MESSAGE_OFF, 0);
  sendmsg(AO_LED_COLOR_GREEN , AO_LED_MESSAGE_OFF, 0);
  sendmsg(AO_LED_COLOR_BLUE  , AO_LED_MESSAGE_OFF, 0);

  msg_event_t event_msg = {0};

  while (true)
  {
    if (pdPASS == xQueueReceive(hao_.hqueue, &event_msg, portMAX_DELAY))
    {
      switch (event_msg)
      {
        case MSG_EVENT_BUTTON_PULSE:
          LOGGER_INFO("led red");
          sendmsg(AO_LED_COLOR_BLUE  , AO_LED_MESSAGE_OFF, 0);
          sendmsg(AO_LED_COLOR_GREEN , AO_LED_MESSAGE_OFF, 0);
          sendmsg(AO_LED_COLOR_RED   , AO_LED_MESSAGE_ON , 0);
          break;
        case MSG_EVENT_BUTTON_SHORT:
          LOGGER_INFO("led green");
          sendmsg(AO_LED_COLOR_BLUE  , AO_LED_MESSAGE_OFF, 0);
          sendmsg(AO_LED_COLOR_RED   , AO_LED_MESSAGE_OFF, 0);
          sendmsg(AO_LED_COLOR_GREEN , AO_LED_MESSAGE_ON , 0);
          break;
        case MSG_EVENT_BUTTON_LONG:
          LOGGER_INFO("led blue");
          sendmsg(AO_LED_COLOR_RED   , AO_LED_MESSAGE_OFF, 0);
          sendmsg(AO_LED_COLOR_GREEN , AO_LED_MESSAGE_OFF, 0);
          sendmsg(AO_LED_COLOR_BLUE  , AO_LED_MESSAGE_ON , 0);
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
  hao_.hqueue = xQueueCreate(QUEUE_LENGTH_, QUEUE_ITEM_SIZE_);
  while(NULL == hao_.hqueue)
  {
    // error
  }

  BaseType_t status;
  status = xTaskCreate(task_, "task_ao_ui", 128, NULL, tskIDLE_PRIORITY, NULL);
  while (pdPASS != status)
  {
    // error
  }
}

/********************** end of file ******************************************/
