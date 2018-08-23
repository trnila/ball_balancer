#include <tim.h>
#include <stm32f1xx_hal_tim.h>
#include <cstring>
#include "stm32f1xx.h"
#include "usart.h"
#include "balancer/ball_balancer.h"
#include "balancer/stm_touch.h"
#include "cmsis_os.h"
#include "portmacro.h"
#include "balancer/comm.h"
#include "adc.h"
#include "balancer/benchmark.h"
#include "measure.h"

extern "C" void controlTask(void const * argument);

STMTouch touch;
Configuration conf;
BallBalancer balancer(&touch, conf);

void set_pwm(uint32_t channel, int us) {
	double t = 1.0 / ((double) HAL_RCC_GetHCLKFreq() / (htim3.Init.Prescaler + 1));
	int pulse = (double) us * (pow(10, -6)) / t;

	configASSERT(pulse < 0xFFFF);
	__HAL_TIM_SET_COMPARE(&htim3, channel, pulse);
}

void controlTask(void const * argument) {
	// initialize pwm for servos
	configASSERT(HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_1) == HAL_OK);
	configASSERT(HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_2) == HAL_OK);
	configASSERT(HAL_ADCEx_Calibration_Start(&hadc1) == HAL_OK);

	configASSERT(xTaskCreate(measureTask, "measure", 512, NULL, tskIDLE_PRIORITY, NULL) == pdTRUE);

	TickType_t ticks = xTaskGetTickCount();
	Measurement measurement{};
	benchmark_init();
	for(;;) {
		benchmark_start(0);

		if(balancer.update(measurement) && !conf.disableServos) {
			set_pwm(TIM_CHANNEL_1, measurement.USX);
			set_pwm(TIM_CHANNEL_2, measurement.USY);
		}

		char buffer[40];
		snprintf(buffer, sizeof(buffer), "%d,%d\r\n", (int) measurement.USX, (int) measurement.USY);
		HAL_UART_Transmit(&huart1, (uint8_t *) buffer, strlen(buffer), HAL_MAX_DELAY);

		//sendCommand(CMD_MEASUREMENT, (char *) &measurement, sizeof(measurement));
		benchmark_stop(0, "measure & control cycle");

		vTaskDelayUntil(&ticks, MEASUREMENT_PERIOD_MS);
	}
}
