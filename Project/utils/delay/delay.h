#include <stdbool.h>
#include <assert.h>
#include <stdint.h>

typedef uint32_t tick_t;
typedef bool bool_t;

typedef struct
{
	tick_t startTime;
	tick_t duration;
	tick_t running;
} delay_t;

void delayInit(delay_t* delay, tick_t duration);
bool_t delayRead(delay_t* delay);
void delayWrite(delay_t* delay, tick_t duration);
void delay_stop( delay_t * delay);
bool_t delayIsRunning(delay_t* delay);
