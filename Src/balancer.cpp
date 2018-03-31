#include <tim.h>
#include <stm32f1xx_hal_tim.h>
#include "stm32f1xx.h"
#include "usart.h"
#include "balancer/ball_balancer.h"
#include "STMTouch.h"
#include "cmsis_os.h"
#include "portmacro.h"
#include "string.h"

extern "C" {
	void controlTask(void const * argument);
};

void set_pwm(uint32_t channel, int us);
void send(Measurement& measurement);

STMTouch touch;
VelocityTracker tracker(&touch);
Configuration conf;
ball_balancer balancer(tracker, conf);

xQueueHandle txqueue;
Measurement txbuffer;

#define MAX_BUF 64
char rxbuffer[MAX_BUF];
int rxpos = 0;
xQueueHandle rxcommands;

struct Frame {
	char buffer[MAX_BUF];
	int size;
};

size_t UnStuffData(const uint8_t *ptr, size_t length, uint8_t *dst) {
	const uint8_t *start = dst, *end = ptr + length;
	uint8_t code = 0xFF, copy = 0;

	for (; ptr < end; copy--) {
		if (copy != 0) {
			*dst++ = *ptr++;
		} else {
			if (code != 0xFF)
				*dst++ = 0;
			copy = code = *ptr++;
			if (code == 0)
				break; /* Source length too long */
		}
	}
	return dst - start;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if(rxbuffer[rxpos] == 0) {
		Frame frame;
		frame.size = rxpos;
		memcpy(frame.buffer, rxbuffer, rxpos);
		xQueueSendFromISR(rxcommands, &frame, NULL);
		rxpos = 0;
	} else {
		rxpos = (rxpos + 1) % MAX_BUF;
	}

	int x = HAL_UART_Receive_IT(&huart1, (uint8_t*) rxbuffer + rxpos, 1);
	configASSERT(x == HAL_OK);
}

void processCommand(char cmd, char* args) {
	if(cmd == 0) {
		balancer.reset();
	} else if(cmd == 1) {
		int x = *(int*) args;
		int y = *(int*) (args + sizeof(int));
		taskENTER_CRITICAL();
		balancer.setTargetPosition(x, y);
		taskEXIT_CRITICAL();
	} else if(cmd == 2) {
		double p = *(double*) args;
		double i = *(double*) (args + sizeof(double));
		double d = *(double*) (args + sizeof(double) * 2);

		taskENTER_CRITICAL();
		conf.const_p = p;
		conf.const_i = i;
		conf.const_d = d;
		taskEXIT_CRITICAL();
	}
}

void handleInput() {
	Frame frame;
	while(xQueueReceive(rxcommands, &frame, 0) == pdTRUE) {
		char buffer[MAX_BUF];

		UnStuffData(reinterpret_cast<const uint8_t *>(frame.buffer), frame.size, reinterpret_cast<uint8_t *>(buffer));
		processCommand(*buffer, buffer + 1);
	}
}

void controlTask(void const * argument) {
	// initialize pwm for servos
	configASSERT(HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_1) == HAL_OK);
	configASSERT(HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_2) == HAL_OK);

	configASSERT(rxcommands = xQueueCreate(5, MAX_BUF));
	configASSERT(HAL_UART_Receive_IT(&huart1, (uint8_t*) &rxbuffer, 1) == HAL_OK);

	configASSERT(txqueue = xQueueCreate(5, sizeof(Measurement)));

	configASSERT(HAL_ADCEx_Calibration_Start(&hadc1) == HAL_OK);

	TickType_t ticks = xTaskGetTickCount();


	/*
	const int STEPS = 25;
	for(;;) {
		for(int i = 0; i <= STEPS; i++) {
			set_pwm(TIM_CHANNEL_1, CENTER_X_US + (sin((float) i / STEPS) * 2 - 1) * 500);
			set_pwm(TIM_CHANNEL_2, CENTER_Y_US + (sin((float) i / STEPS) * 2 - 1) * 500);
			osDelay(20);
		}

		for(int i = 0; i <= STEPS; i++) {
			set_pwm(TIM_CHANNEL_2, CENTER_X_US + (sin((float) (STEPS - i) / STEPS) * 2 - 1) * 500);
			set_pwm(TIM_CHANNEL_1, CENTER_Y_US + (sin((float) (STEPS - i) / STEPS) * 2 - 1) * 500);
			osDelay(20);
		}
	}

	set_pwm(TIM_CHANNEL_1, 1500);
	set_pwm(TIM_CHANNEL_2, 1500);
	for(;;);
	*/

	Measurement measurement;
	for(;;) {
		HAL_GPIO_WritePin(DEBUG_GPIO_Port, DEBUG_Pin, GPIO_PIN_SET);
		if(balancer.update(measurement)) {
			set_pwm(TIM_CHANNEL_1, measurement.USX);
			set_pwm(TIM_CHANNEL_2, measurement.USY);
		}

		UBaseType_t waiting = uxQueueMessagesWaiting(txqueue);
		if(waiting == 0) {
			txbuffer = measurement;
			configASSERT(HAL_UART_Transmit_DMA(&huart1, (uint8_t *) &txbuffer, sizeof(txbuffer)) == HAL_OK);
		} else {
			xQueueSend(txqueue, &measurement, 0);
		}

		HAL_GPIO_WritePin(DEBUG_GPIO_Port, DEBUG_Pin, GPIO_PIN_RESET);

		handleInput();

		vTaskDelayUntil(&ticks, MEASUREMENT_PERIOD_MS);
	}
}

void set_pwm(uint32_t channel, int us) {
	double t = 1.0 / (HAL_RCC_GetHCLKFreq() / (htim3.Init.Prescaler + 1));
	int pulse = (double) us * (pow(10, -6)) / t;

	configASSERT(pulse < 0xFFFF);
	__HAL_TIM_SET_COMPARE(&htim3, channel, pulse);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if(xQueueReceiveFromISR(txqueue, &txbuffer, nullptr) == pdTRUE) {
		configASSERT(HAL_UART_Transmit_DMA(&huart1, (uint8_t *) &txbuffer, sizeof(txbuffer)) == HAL_OK);
	}
}
