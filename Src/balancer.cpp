#include "stm32f1xx.h"
#include "usart.h"
#include "BallBalancer.h"
#include "STMTouch.h"
#include "cmsis_os.h"
#include "string.h"
#include <tim.h>

extern "C" {
	void controlTask(void const * argument);
};

void pwm_set_period(TIM_HandleTypeDef *timer, uint16_t period) {
	timer->Init.Period = period;
	configASSERT(HAL_TIM_PWM_Init(timer) == HAL_OK);
}

void pwm_set_duty(TIM_HandleTypeDef *timer, uint32_t channel, uint16_t pulse) {
	configASSERT(HAL_TIM_PWM_Stop(timer, channel) == HAL_OK);

	TIM_OC_InitTypeDef conf;
	conf.Pulse = pulse;
	conf.OCMode = TIM_OCMODE_PWM1;
	conf.OCPolarity = TIM_OCPOLARITY_HIGH;
	conf.OCFastMode = TIM_OCFAST_DISABLE;
	configASSERT(HAL_TIM_PWM_ConfigChannel(timer, &conf, channel) == HAL_OK);
	configASSERT(HAL_TIM_PWM_Start(timer, channel) == HAL_OK);
}


void writeServos(double x, double y) {
	pwm_set_duty(&htim3, TIM_CHANNEL_1, (uint16_t ) DUTY_MS * x);
	//pwm_set_duty(&htim3, TIM_CHANNEL_2, (uint16_t ) DUTY_MS * y);
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

	/* USER CODE BEGIN controlTask */
	pwm_set_period(&htim3, 0xFFFF);
	for(;;) {
		pwm_set_duty(&htim3, TIM_CHANNEL_2, 0xFFFF);
		osDelay(20);
	}

	for(;;)
	{


		HAL_GPIO_WritePin(DEBUG_GPIO_Port, DEBUG_Pin, GPIO_PIN_SET);
		balancer.update();
		HAL_GPIO_WritePin(DEBUG_GPIO_Port, DEBUG_Pin, GPIO_PIN_RESET);
		osDelay(20);
	}
	/* USER CODE END controlTask */
}