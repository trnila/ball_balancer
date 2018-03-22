#pragma once

#include <cmsis_os.h>
#include "balancer/itouch.h"

extern "C" {
	#include "stm32f1xx.h"
	#include "adc.h"
	extern osThreadId controlHandle;
	extern BaseType_t pxHigherPriorityTaskWoken;
}

#define SAMPLES_NUM 32

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
	vTaskNotifyGiveFromISR(controlHandle, &pxHigherPriorityTaskWoken);
}


class STMTouch: public itouch {
public:
	virtual	void read(int &X, int &Y) {
		ADC_ChannelConfTypeDef sConfig;
		
		// measure Y
		pinMode(TOUCH_YP_Pin, GPIO_MODE_INPUT);
		pinMode(TOUCH_YM_Pin, GPIO_MODE_INPUT);
		pinMode(TOUCH_XP_Pin, GPIO_MODE_OUTPUT_PP);
		pinMode(TOUCH_XM_Pin, GPIO_MODE_OUTPUT_PP);

		digitalWrite(TOUCH_XP_Pin, GPIO_PIN_SET);
		digitalWrite(TOUCH_XM_Pin, GPIO_PIN_RESET);

		sConfig.Channel = ADC_CHANNEL_0;
		sConfig.Rank = 1;
		sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
		configASSERT(HAL_ADC_ConfigChannel(&hadc1, &sConfig) == HAL_OK);

		Y = measureAxis();

		// ================== X AXIS ================
		pinMode(TOUCH_XP_Pin, GPIO_MODE_INPUT);
		pinMode(TOUCH_XM_Pin, GPIO_MODE_INPUT);
		pinMode(TOUCH_YP_Pin, GPIO_MODE_OUTPUT_PP);
		pinMode(TOUCH_YM_Pin, GPIO_MODE_OUTPUT_PP);

		digitalWrite(TOUCH_YP_Pin, GPIO_PIN_SET);
		digitalWrite(TOUCH_YM_Pin, GPIO_PIN_RESET);

		sConfig.Channel = ADC_CHANNEL_1;
		sConfig.Rank = 1;
		sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
		configASSERT(HAL_ADC_ConfigChannel(&hadc1, &sConfig) == HAL_OK);

		X = measureAxis();
	}

	void pinMode(int pin, int mode) {
		GPIO_InitTypeDef GPIO_InitStruct;
		GPIO_InitStruct.Pin = pin;
		GPIO_InitStruct.Mode = mode;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	}

	void digitalWrite(int pin, GPIO_PinState state) {
		HAL_GPIO_WritePin(GPIOA, pin, state);
	}

	int measureAxis() {
		configASSERT(HAL_ADC_Start_DMA(&hadc1, (uint32_t*) &samples, SAMPLES_NUM) == HAL_OK);
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		for(int i = 0; i < SAMPLES_NUM; i++) {
			for(int j = 0; j < SAMPLES_NUM - 1; j++) {
				if(samples[j] > samples[j + 1]) {
					uint16_t tmp = samples[j];
					samples[j] = samples[j + 1];
					samples[j + 1] = tmp;
				}
			}
		}

		int sum = 0;
		int take = 4;
		for(int i = SAMPLES_NUM / 2 - take; i < SAMPLES_NUM / 2 + take; i++) {
			sum += samples[i];
		}

		return sum / (take * 2);
	}

private:
	uint16_t samples[SAMPLES_NUM];
};