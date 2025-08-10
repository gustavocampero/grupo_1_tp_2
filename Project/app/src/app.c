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
#include "dwt.h"
#include "board.h"

#include "task_button.h"
#include "task_led.h"
#include "task_ui.h"

/* Logger (pub/sub) */
#include "logger.h"
#include "logger_sink_uart.h"

/* UART AO + port HW */
#include "ao_uart.h"
#include "port_uart.h"

/********************** macros and definitions *******************************/


/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/

/********************** external data declaration *****************************/

/********************** external functions definition ************************/
void app_init(void)
{
      /* 1) Inicialización de HW específico del UART (clocks/GPIO/USART/DMA deben
           estar configurados por CubeMX; acá solo confirmamos/afinamos si hace falta) */
    port_uart_hw_init();

    /* 2) Crear el AO dueño del UART (cola/stream + task) */
    if (!ao_uart_create()) {
        /* Si querés, podés hacer un fallback o quedarte en error */
        // for(;;);
    }

    /* 3) Inicializar el logger y suscribir el sink hacia el AO UART */
    log_init();
    /* Enviará solo INFO+ por UART. Cambiá LOG_INFO por LOG_DEBUG si querés todo. */
    (void)log_subscribe(logger_sink_uart, LOG_INFO, NULL);

    /* 4) Logs de smoke test */
    log_printf(LOG_INFO,  "Logger listo. AO UART activo.\r\n");
    log_printf(LOG_WARN,  "Ejemplo WARN: nivel de batería bajo (simulado).\r\n");
    log_printf(LOG_ERROR, "Ejemplo ERROR: código=%d (simulado).\r\n", 42);
  //BaseType_t status;

  // Primero se inicializa el UI ya que controla los LEDs
  //LOGGER_INFO("Initializing UI...");
  //ao_ui_init();

  // Crear la tarea de botón con alta prioridad para una respuesta rápida ya que no será un OA
  //LOGGER_INFO("Creating button task...");
  //status = xTaskCreate(task_button, "task_button", 128, NULL, tskIDLE_PRIORITY + 2, NULL);
  //if (pdPASS != status)
  //{
    //LOGGER_ERROR("Failed to create button task");
    //while(1);
  //}

  //LOGGER_INFO("Initializing cycle counter...");
  cycle_counter_init();
  
  //LOGGER_INFO("Application initialization complete");
}

/********************** end of file ******************************************/
