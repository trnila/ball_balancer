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

extern "C" void controlTask(void const * argument);

STMTouch touch;
VelocityTracker tracker(&touch);
Configuration conf;
BallBalancer balancer(tracker, conf);

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

	TickType_t ticks = xTaskGetTickCount();
	Measurement measurement{};
	benchmark_init();
	for(;;) {
		benchmark_start(0);

		HAL_GPIO_WritePin(DEBUG_GPIO_Port, DEBUG_Pin, GPIO_PIN_SET);
		if(balancer.update(measurement) && !conf.disableServos) {
			set_pwm(TIM_CHANNEL_1, measurement.USX);
			set_pwm(TIM_CHANNEL_2, measurement.USY);
		}

		sendCommand(CMD_MEASUREMENT, (char *) &measurement, sizeof(measurement));
		HAL_GPIO_WritePin(DEBUG_GPIO_Port, DEBUG_Pin, GPIO_PIN_RESET);
		benchmark_stop(0, "measure & control cycle");

		vTaskDelayUntil(&ticks, MEASUREMENT_PERIOD_MS);
	}
}
