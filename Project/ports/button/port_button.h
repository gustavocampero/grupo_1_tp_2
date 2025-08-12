#ifndef PORT_BUTTON_H_
#define PORT_BUTTON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "FreeRTOS.h"
#include "timers.h"

/* Estos macros deben existir en tu board.h o main.h:
 *   #define BUTTON_PORT   GPIOA
 *   #define BUTTON_PIN    GPIO_PIN_0
 * Y opcionalmente:
 *   #define BUTTON_ACTIVE_LOW 1   // si el botón es activo-bajo
 */

void port_button_register_debounce_timer(TimerHandle_t tmr);
bool port_button_read(void);   /* true = presionado */

#ifdef __cplusplus
}
#endif
#endif /* PORT_BUTTON_H_ */
