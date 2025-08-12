#ifndef MSG_EVENT_H_
#define MSG_EVENT_H_

// Tipos de eventos del UI
typedef enum {
    MSG_EVENT_BUTTON_PULSE,    // Botón presionado brevemente - Alta prioridad
    MSG_EVENT_BUTTON_SHORT,    // Botón presionado tiempo medio - Media prioridad
    MSG_EVENT_BUTTON_LONG,     // Botón presionado largo tiempo - Baja prioridad
    MSG_EVENT__N
} msg_event_t;

#endif /* MSG_EVENT_H_ */
