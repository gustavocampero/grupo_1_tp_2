#include <API_delay.h>
#include <stm32f4xx_hal.h>

void delayInit(delay_t* delay, tick_t duration){
	assert(delay);

	delay->startTime = HAL_GetTick();
	delay->duration = duration;
	delay->running = false;
}

bool_t delayRead(delay_t* delay){
	assert(delay);

	tick_t Current_time = HAL_GetTick();
	bool_t Done = false;

	if(!delay->running){
		delay->startTime = Current_time;
		delay->running = true;
	}else{
		// Done is TRUE when the duration has elapsed
		// Done is FALSE when theres time remaining
		Done = (Current_time - delay->startTime) >= delay->duration;
		// If it's done, it should stop running
		delay->running = !Done;
	}
	return Done;
}

void delayWrite( delay_t * delay, tick_t duration ){
	assert(delay);

	delay->duration = duration;
}

void delay_stop( delay_t * delay){
	assert(delay);

	delay->duration = 0;
	delay->running = false;
}

bool_t delayIsRunning(delay_t* delay){
	return delay->running;
}
