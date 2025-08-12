/**
 * @file port_button.c
 * @brief Implementación específica del hardware para la lectura del botón.
 *
 * Este archivo contiene la función que interactúa con HAL para leer el estado físico del botón.
 */

#include "port_button.h"
#include "stm32f4xx_hal.h"  /* Para GPIO_PinState y HAL_GPIO_ReadPin */

// Configuración de la placa: el botón es activo en bajo
#define BUTTON_PULL_LOW     true        ///< true si el botón es activo-bajo

/**
 * Lee el estado actual del botón.
 *
 * Utiliza HAL_GPIO_ReadPin para leer el pin físico del botón. Si el botón es activo-bajo,
 * invierte la lógica para que devuelva true cuando esté presionado.
 *
 * @return true si el botón está presionado, false si está liberado.
 */
bool port_button_read(void) {
    // Leemos el estado lógico del pin del botón
    GPIO_PinState pin_state = HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN);

    // Si el botón es activo-bajo (pull-down), consideramos presionado cuando el pin está en bajo (RESET)
    // Si fuera activo-alto, consideraríamos presionado cuando el pin esté en alto (SET)
    // El operador ternario permite devolver directamente true o false según esta lógica
    return BUTTON_PULL_LOW ? (pin_state == GPIO_PIN_RESET) : (pin_state == GPIO_PIN_SET);
}
