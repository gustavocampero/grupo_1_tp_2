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

#include "main.h"
#include "cmsis_os.h"
#include "logger.h"
#include "logger_sink_uart.h"
#include "ao_uart.h"
#include "port_uart.h"
#include "dwt.h"
#include "board.h"

#include "task_button.h"
#include "task_led.h"
#include "task_ui.h"

/********************** macros and definitions *******************************/


/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/

/********************** external data declaration *****************************/


/********************** external functions definition ************************/
void app_init(void)
{
  // 1) Hardware initialization for UART (clocks/GPIO/USART/DMA should be configured by CubeMX)
  port_uart_hw_init();

  // 2) Create AO UART (queue/stream + task)
  ao_uart_create();

  // 3) Initialize logger sink mutex
  logger_sink_uart_init();

  // 4) Initialize logger and subscribe UART sink
  log_init();
  (void)log_subscribe(logger_sink_uart, LOG_INFO, NULL);

  // 5) Initialize cycle counter
  cycle_counter_init();

  // 6) Initialize application objects
  ao_ui_init();
  ao_led_init();

  // 7) Create application tasks
  static TaskHandle_t button_task_handle = NULL;
  BaseType_t status;

  status = xTaskCreate(task_button, "task_button", 512, NULL, tskIDLE_PRIORITY + 3, &button_task_handle);
  if (pdPASS != status)
  {
    LOGGER_ERROR("Failed to create button task!");
    Error_Handler();
  }
  else
  {
    LOGGER_INFO("Button task created successfully");
  }

  LOGGER_INFO("app init completed");
}

/********************** end of file ******************************************/
