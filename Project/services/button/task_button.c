/********************** inclusions *******************************************/
#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "task_ui.h"        // ao_ui_send_event(...)
#include "port_button.h"    // port_button_read(), register timer

/********************** macros and definitions *******************************/

#define DEBOUNCE_MS      (30U)

#define PULSE_MIN_MS     (200U)
#define PULSE_MAX_MS     (1000U)
#define SHORT_MIN_MS     (1000U)
#define SHORT_MAX_MS     (2000U)
#define LONG_MIN_MS      (2000U)

/* señales para la worker (por notify bits) */
#define BTN_SIG_PRESS    (1UL << 0)
#define BTN_SIG_RELEASE  (1UL << 1)

/********************** internal data declaration ****************************/
static TimerHandle_t s_btnDebounceTmr = NULL;
static TaskHandle_t  s_btnWorkerHandle = NULL;

/********************** internal functions declaration ***********************/
static void vBtnDebounceCb(TimerHandle_t t);
static void vButtonWorker(void *arg);
static inline void notify_worker(uint32_t sig);

/********************** external functions definition ************************/
void task_button_init(void)
{
  if (s_btnDebounceTmr == NULL) {
    s_btnDebounceTmr = xTimerCreate("btn_db",
                                    pdMS_TO_TICKS(DEBOUNCE_MS),
                                    pdFALSE, /* one-shot */
                                    NULL,
                                    vBtnDebounceCb);
  }
  /* habilita que la ISR del port pueda (re)iniciar el timer */
  port_button_register_debounce_timer(s_btnDebounceTmr);
}

/********************** internal functions definition ************************/
/* Callback de timer: se ejecuta en contexto de task (no ISR). */
static void vBtnDebounceCb(TimerHandle_t t)
{
  (void)t;
  const bool pressed = port_button_read();

  if (pressed) {
    if (s_btnWorkerHandle == NULL) {
      /* crear worker efímera para esta pulsación */
      xTaskCreate(vButtonWorker, "btn_w",
                  256, NULL, tskIDLE_PRIORITY + 2, &s_btnWorkerHandle);
    }
    notify_worker(BTN_SIG_PRESS);
  } else {
    notify_worker(BTN_SIG_RELEASE);
  }
}

static inline void notify_worker(uint32_t sig)
{
  if (s_btnWorkerHandle) {
    xTaskNotify(s_btnWorkerHandle, sig, eSetBits);
  }
}

/* Worker: mide PRESS→RELEASE, clasifica, envía evento y se borra */
static void vButtonWorker(void *arg)
{
  (void)arg;
  uint32_t bits = 0;
  TickType_t t_press = 0;

  /* esperar primer PRESS */
  xTaskNotifyWait(0, 0xFFFFFFFFu, &bits, portMAX_DELAY);
  if ((bits & BTN_SIG_PRESS) == 0) {
    s_btnWorkerHandle = NULL;
    vTaskDelete(NULL);
  }
  t_press = xTaskGetTickCount();

  /* esperar RELEASE (ignorar PRESS repetidos) */
  for (;;) {
    xTaskNotifyWait(0, 0xFFFFFFFFu, &bits, portMAX_DELAY);
    if (bits & BTN_SIG_RELEASE) break;
  }

  const TickType_t t_release = xTaskGetTickCount();
  const uint32_t dt_ms = (uint32_t)((t_release - t_press) * 1000 / configTICK_RATE_HZ);

  ao_ui_event_t ev = MSG_EVENT_BUTTON_NONE;
  if (dt_ms >= PULSE_MIN_MS && dt_ms < PULSE_MAX_MS) {
    ev = MSG_EVENT_BUTTON_PULSE;
  } else if (dt_ms >= SHORT_MIN_MS && dt_ms < SHORT_MAX_MS) {
    ev = MSG_EVENT_BUTTON_SHORT_PRESS;
  } else if (dt_ms >= LONG_MIN_MS) {
    ev = MSG_EVENT_BUTTON_LONG_PRESS;
  } else {
    /* menor a 200 ms: ruido o tap muy corto -> ignorar */
  }

  if (ev != MSG_EVENT_BUTTON_NONE) {
    (void)ao_ui_send_event(ev);
  }

  s_btnWorkerHandle = NULL;
  vTaskDelete(NULL);  /* muere la worker */
}

/********************** end of file ******************************************/
