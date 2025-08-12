// Standard includes
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// Project includes
#include "priority_queue.h"
#include "logger.h"

// FreeRTOS includes
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// Implementación de las funciones de la cola de prioridades
void priority_queue_init(priority_queue_t* pq, 
                        uint8_t* high_buf, 
                        uint8_t* medium_buf, 
                        uint8_t* low_buf, 
                        size_t queue_size) {
    assert(pq != NULL);
    assert(high_buf != NULL && medium_buf != NULL && low_buf != NULL);
    
    // Inicializar buffers
    pq->high_buffer = high_buf;
    pq->medium_buffer = medium_buf;
    pq->low_buffer = low_buf;
    
    ring_buffer_init(&pq->high_priority, high_buf, queue_size);
    ring_buffer_init(&pq->medium_priority, medium_buf, queue_size);
    ring_buffer_init(&pq->low_priority, low_buf, queue_size);
    
    // Crear mutex para cada cola
    pq->mutex_high = xSemaphoreCreateMutex();
    pq->mutex_medium = xSemaphoreCreateMutex();
    pq->mutex_low = xSemaphoreCreateMutex();
    pq->queue_access_mutex = xSemaphoreCreateMutex();
    
    // Verificar creación exitosa de mutex
    assert(pq->mutex_high != NULL);
    assert(pq->mutex_medium != NULL);
    assert(pq->mutex_low != NULL);
    assert(pq->queue_access_mutex != NULL);
}

bool priority_queue_send(priority_queue_t* pq, priority_queue_msg_t* msg) 
{
    bool success = false;

    if (pq == NULL || msg == NULL) {
        return false;
    }
    
    if (xSemaphoreTake(pq->queue_access_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return false;
    }

    ring_buffer_t* target_queue = NULL;
    SemaphoreHandle_t target_mutex = NULL;
    
    // Seleccionar cola basado en prioridad
    switch (msg->priority) {
        case PRIORITY_HIGH:
            target_queue = &pq->high_priority;
            target_mutex = pq->mutex_high;
            break;
        case PRIORITY_MEDIUM:
            target_queue = &pq->medium_priority;
            target_mutex = pq->mutex_medium;
            break;
        case PRIORITY_LOW:
            target_queue = &pq->low_priority;
            target_mutex = pq->mutex_low;
            break;
        default:
            xSemaphoreGive(pq->queue_access_mutex);
            return false;
    }
    
    // Intentar obtener mutex específico de la cola
    if (xSemaphoreTake(target_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        // Si la cola está llena, descartamos el mensaje más antiguo
        if (ring_buffer_is_full(target_queue)) {
            priority_queue_msg_t old_msg;
            ring_buffer_read_bytes(target_queue, (uint8_t*)&old_msg, sizeof(priority_queue_msg_t));
            // Removed excessive logging that was causing corruption
        }
        
        // Escribir el nuevo mensaje
        ring_buffer_write_bytes(target_queue, (uint8_t*)msg, sizeof(priority_queue_msg_t));
        success = true;
        
        xSemaphoreGive(target_mutex);
    }
    
    xSemaphoreGive(pq->queue_access_mutex);
    return success;
}

bool priority_queue_receive(priority_queue_t* pq, priority_queue_msg_t* msg) {
    if (pq == NULL || msg == NULL) {
        return false;
    }
    
    if (xSemaphoreTake(pq->queue_access_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return false;
    }
    
    bool success = false;
    
    // Intentar leer de las colas en orden de prioridad
    if (xSemaphoreTake(pq->mutex_high, pdMS_TO_TICKS(100)) == pdTRUE) {
        if (!ring_buffer_is_empty(&pq->high_priority)) {
            ring_buffer_read_bytes(&pq->high_priority, (uint8_t*)msg, sizeof(priority_queue_msg_t));
            success = true;
        }
        xSemaphoreGive(pq->mutex_high);
    }
    
    if (!success && xSemaphoreTake(pq->mutex_medium, pdMS_TO_TICKS(100)) == pdTRUE) {
        if (!ring_buffer_is_empty(&pq->medium_priority)) {
            ring_buffer_read_bytes(&pq->medium_priority, (uint8_t*)msg, sizeof(priority_queue_msg_t));
            success = true;
        }
        xSemaphoreGive(pq->mutex_medium);
    }
    
    if (!success && xSemaphoreTake(pq->mutex_low, pdMS_TO_TICKS(100)) == pdTRUE) {
        if (!ring_buffer_is_empty(&pq->low_priority)) {
            ring_buffer_read_bytes(&pq->low_priority, (uint8_t*)msg, sizeof(priority_queue_msg_t));
            success = true;
        }
        xSemaphoreGive(pq->mutex_low);
    }
    
    xSemaphoreGive(pq->queue_access_mutex);
    return success;
}

bool priority_queue_is_empty(priority_queue_t* pq) {
    return ring_buffer_is_empty(&pq->high_priority) &&
           ring_buffer_is_empty(&pq->medium_priority) &&
           ring_buffer_is_empty(&pq->low_priority);
}

bool priority_queue_is_full(priority_queue_t* pq) {
    return ring_buffer_is_full(&pq->high_priority) &&
           ring_buffer_is_full(&pq->medium_priority) &&
           ring_buffer_is_full(&pq->low_priority);
}
