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

/********************** external data definition *****************************/

extern ao_led_handle_t led_red;
extern ao_led_handle_t led_green;
extern ao_led_handle_t led_blue;

static memory_pool_t memory_pool_;
static uint8_t memory_pool_memory_[MEMORY_POOL_SIZE(MEMORY_POOL_NBLOCKS, MEMORY_POOL_BLOCK_SIZE)];

memory_pool_t* const hmp = &memory_pool_;

/********************** internal functions definition ************************/

static void callback_(ao_led_message_t* pmsg)
{
	memory_pool_block_put(hmp, (void*)pmsg);
    LOGGER_INFO("Memoria liberada desde button");
    msg_wip_--;
    LOGGER_INFO("Mensajes en proceso: %d", msg_wip_);

}

static void initLed(ao_led_handle_t *hand)
{
	ao_led_message_t* led_msg_init = (ao_led_message_t*)memory_pool_block_get(hmp);
	if(NULL != pmsg)
	{
	  led_msg_init->callback = callback_;
	  led_msg_init->id = id++;
	  led_msg_init->action = AO_LED_MESSAGE_OFF;
	  led_msg_init->value = 1000;
	  ao_led_send(hand, &led_msg_init);
	}
}

static void task_(void *argument)
{
  memory_pool_init(hmp, memory_pool_memory_, MEMORY_POOL_NBLOCKS, MEMORY_POOL_BLOCK_SIZE);
  int id = 0;

  initLed(&led_red);
  initLed(&led_green);
  initLed(&led_blue);

  while (true)
  {

    ao_led_message_t led_msg;
    led_msg.callback = callback_;
    led_msg.id = ++id;
    led_msg.action = AO_LED_MESSAGE_BLINK;
    led_msg.value = 1000;

    msg_event_t event_msg;

    if (pdPASS == xQueueReceive(hao_.hqueue, &event_msg, portMAX_DELAY))
    {
      switch (event_msg)
      {
        case MSG_EVENT_BUTTON_PULSE:
          LOGGER_INFO("led red");
          ao_led_send(&led_red, &led_msg);
          break;
        case MSG_EVENT_BUTTON_SHORT:
          LOGGER_INFO("led green");
          ao_led_send(&led_green, &led_msg);
          break;
        case MSG_EVENT_BUTTON_LONG:
          LOGGER_INFO("led blue");
          ao_led_send(&led_blue, &led_msg);
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
