#include <tim.h>
#include <stm32f1xx_hal_tim.h>
#include "stm32f1xx.h"
#include "usart.h"
#include "BallBalancer.h"
#include "STMTouch.h"
#include "cmsis_os.h"
#include "string.h"
//#include <tim.h>

extern "C" {
	void controlTask(void const * argument);
};

void set_pwm(uint32_t channel, double val) {
	TIM_OC_InitTypeDef conf;
	conf.Pulse = val * htim3.Init.Period;
	conf.OCMode = TIM_OCMODE_PWM1;
	conf.OCPolarity = TIM_OCPOLARITY_HIGH;
	conf.OCFastMode = TIM_OCFAST_ENABLE;
	configASSERT(HAL_TIM_PWM_ConfigChannel(&htim3, &conf, channel) == HAL_OK);
	configASSERT(HAL_TIM_PWM_Start_IT(&htim3, channel) == HAL_OK);
}

void writeServos(double x, double y) {
	set_pwm(TIM_CHANNEL_1, x);
	set_pwm(TIM_CHANNEL_2, y);
}

void send(Measurement& measurement) {
	HAL_UART_Transmit(&huart1, (uint8_t*) &measurement, sizeof(measurement), HAL_MAX_DELAY);
}


STMTouch touch;
Filter filter(&touch);
VelocityTracker tracker(&touch);
Configuration conf;
BallBalancer balancer(tracker, conf, writeServos, send);


void controlTask(void const * argument) {
	// initialize pwm for servos
	configASSERT(HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_1) == HAL_OK);
	configASSERT(HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_2) == HAL_OK);

	TickType_t ticks = xTaskGetTickCount();

	for(;;) {
		HAL_GPIO_WritePin(DEBUG_GPIO_Port, DEBUG_Pin, GPIO_PIN_SET);
		balancer.update();
		HAL_GPIO_WritePin(DEBUG_GPIO_Port, DEBUG_Pin, GPIO_PIN_RESET);

		vTaskDelayUntil(&ticks, MEASUREMENT_PERIOD_MS);
	}
}
