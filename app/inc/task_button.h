/*
 * Copyright (c) 2023 Sebastian Bedin <sebabedin@gmail.com>.
 * All rights reserved.

#ifndef TASK_BUTTON_H_
#define TASK_BUTTON_H_

#include <stdint.h>
#include <stdbool.h>

// Button states
typedef enum {
    BUTTON_STATE_NONE,
    BUTTON_STATE_PULSE,
    BUTTON_STATE_SHORT,
    BUTTON_STATE_LONG
} button_state_t;

// Button event structure
typedef struct {
    button_state_t state;
    uint32_t timestamp;
} button_event_t;
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

#ifndef TASK_BUTTTON_H_
#define TASK_BUTTTON_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusions *******************************************/

/********************** macros ***********************************************/

/********************** typedef **********************************************/

typedef enum {
    BUTTON_STATE_NONE,    // No presionado o < 200ms
    BUTTON_STATE_PULSE,   // 200ms - 1000ms  -> Alta prioridad
    BUTTON_STATE_SHORT,   // 1000ms - 2000ms -> Media prioridad
    BUTTON_STATE_LONG,    // > 2000ms        -> Baja prioridad
} button_state_t;

typedef struct {
    button_state_t state;
    uint32_t timestamp;
} button_event_t;

/********************** external data declaration ****************************/

/********************** external functions declaration ***********************/

void task_button(void* argument);

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* TASK_BUTTTON_H_ */
/********************** end of file ******************************************/

