#include <string.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "portmacro.h"
#include "task.h"
#include "adc.h"
#include "usart.h"
#include <malloc.h>
#include <math.h>

struct running_avg {
	uint8_t index;
	uint8_t size;
	uint16_t *data;

	uint16_t prev;
	int fails;
};

void running_avg_init(struct running_avg *avg, uint8_t size) {
	avg->index = 0;
	avg->size = size;
	avg->data = malloc(sizeof(uint16_t) * size);
	avg->fails = 0;
	configASSERT(avg->data != NULL);
}

void running_avg_add(struct running_avg *avg, uint16_t num) {
	if(avg->fails < 20 && (num < 200 || abs(num - avg->prev) > 50)) {
		avg->fails++;
		num = avg->prev;
	} else {
		avg->fails = 0;
		avg->prev = num;
	}

	avg->data[avg->index] = num;
	avg->index = (avg->index + 1) % avg->size;

	avg->prev = num;
}

uint16_t running_avg_calculate(struct running_avg *avg) {
	int sum = 0;
	for(int i = 0; i < avg->size; i++) {
		sum += avg->data[i];
	}

	return sum / avg->size;
}

struct running_avg avg_x, avg_y;

int X, Y;

static void pinMode(uint32_t pin, uint32_t mode) {
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = pin;
	GPIO_InitStruct.Mode = mode;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}


static void digitalWrite(int pin, GPIO_PinState state) {
	HAL_GPIO_WritePin(GPIOA, pin, state);
}

static int adcRead(int channel) {
	ADC_ChannelConfTypeDef sConfig;
	sConfig.Channel = channel;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
	configASSERT(HAL_ADC_ConfigChannel(&hadc1, &sConfig) == HAL_OK);

	configASSERT(HAL_ADC_Start(&hadc1) == HAL_OK);
	configASSERT(HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) == HAL_OK);
	int val = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);
	return val;
}


void measureTask(void *argument) {
	running_avg_init(&avg_x, 10);
	running_avg_init(&avg_y, 10);

	for (;;) {
		// measure X
		pinMode(TOUCH_YM_Pin, GPIO_MODE_OUTPUT_PP);
		pinMode(TOUCH_YP_Pin, GPIO_MODE_OUTPUT_PP);
		digitalWrite(TOUCH_YM_Pin, GPIO_PIN_SET);
		digitalWrite(TOUCH_YP_Pin, GPIO_PIN_RESET);
		pinMode(TOUCH_XM_Pin, GPIO_MODE_INPUT);
		pinMode(TOUCH_XP_Pin, GPIO_MODE_INPUT);

		vTaskDelay(1);
		uint16_t measured_x = adcRead(ADC_CHANNEL_1);
		running_avg_add(&avg_x, measured_x);

		// measure Y
		pinMode(TOUCH_XM_Pin, GPIO_MODE_OUTPUT_PP);
		pinMode(TOUCH_XP_Pin, GPIO_MODE_OUTPUT_PP);
		digitalWrite(TOUCH_XM_Pin, GPIO_PIN_SET);
		digitalWrite(TOUCH_XP_Pin, GPIO_PIN_RESET);
		pinMode(TOUCH_YM_Pin, GPIO_MODE_INPUT);
		pinMode(TOUCH_YP_Pin, GPIO_MODE_INPUT);

		vTaskDelay(1);
		uint16_t measured_y = adcRead(ADC_CHANNEL_0);
		running_avg_add(&avg_y, measured_y);

		portENTER_CRITICAL();
		X = running_avg_calculate(&avg_x);
		Y = running_avg_calculate(&avg_y);
		portEXIT_CRITICAL();

		/*char buffer[40];
		snprintf(buffer, sizeof(buffer), "%d,%d,%d,%d\r\n", measured_x, measured_y, X, Y);
		HAL_UART_Transmit(&huart1, (uint8_t *) buffer, strlen(buffer), HAL_MAX_DELAY);
		 */

	}
}