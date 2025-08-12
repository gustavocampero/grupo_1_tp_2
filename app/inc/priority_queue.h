#ifndef PRIORITY_QUEUE_H_
#define PRIORITY_QUEUE_H_

#include <stdint.h>
#include <stdbool.h>
#include "ring_buffer.h"
#include "task_button.h"

// Definición de prioridades
typedef enum {
    PRIORITY_HIGH,    // Para BUTTON_STATE_PULSE
    PRIORITY_MEDIUM,  // Para BUTTON_STATE_SHORT
    PRIORITY_LOW,     // Para BUTTON_STATE_LONG
} priority_level_t;

// Estructura para mensajes en la cola de prioridades
typedef struct {
    priority_level_t priority;
    uint32_t timestamp;
    button_state_t state;
} priority_queue_msg_t;

// Estructura de la cola de prioridades
typedef struct {
    ring_buffer_t high_priority;
    ring_buffer_t medium_priority;
    ring_buffer_t low_priority;
    uint8_t* high_buffer;
    uint8_t* medium_buffer;
    uint8_t* low_buffer;
    SemaphoreHandle_t mutex_high;
    SemaphoreHandle_t mutex_medium;
    SemaphoreHandle_t mutex_low;
    SemaphoreHandle_t queue_access_mutex; // Para proteger acceso general a la cola
} priority_queue_t;

// Funciones de la cola de prioridades
void priority_queue_init(priority_queue_t* pq, 
                        uint8_t* high_buf, 
                        uint8_t* medium_buf, 
                        uint8_t* low_buf, 
                        size_t queue_size);

bool priority_queue_send(priority_queue_t* pq, priority_queue_msg_t* msg);
bool priority_queue_receive(priority_queue_t* pq, priority_queue_msg_t* msg);
bool priority_queue_is_empty(priority_queue_t* pq);
bool priority_queue_is_full(priority_queue_t* pq);

#endif // PRIORITY_QUEUE_H_
