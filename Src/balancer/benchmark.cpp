#include <stm32f1xx.h>
#include <FreeRTOS.h>
#include <task.h>
#include <portmacro.h>
#include <FreeRTOSConfig.h>
#include "tim.h"
#include "balancer/benchmark.h"

unsigned int counter;
unsigned int timers[MAX_TIMERS];

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	counter++;
}

void benchmark_init() {
	htim2.Init.Period = RESOLUTION_US * CLOCK_MHZ;
	if (HAL_TIM_PWM_Init(&htim2) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

	HAL_TIM_Base_Start_IT(&htim2);
}

void benchmark_start(int id) {
	configASSERT(id < MAX_TIMERS);
	timers[id] = counter;
}

void benchmark_stop(int id, const char *msg) {
	configASSERT(id < MAX_TIMERS);

	int current = counter;

	int elapsed;
	if(current > timers[id]) {
		elapsed = current - timers[id];
	} else {
		elapsed = UINT_MAX - timers[id] + current;
	}

	printf("%s: %d us\n", msg, elapsed * RESOLUTION_US);
}