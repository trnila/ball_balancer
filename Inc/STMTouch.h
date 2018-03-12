#pragma once
#include "ITouch.h"

extern "C" {
	#include "stm32f1xx.h"
	#include "adc.h"

#define configASSERT( x ) if ((x) == 0) {asm("bkpt #1"); for( ;; );}
}

#define SAMPLES_NUM 16

class STMTouch: public ITouch {
public:
	virtual void read(int &X, int &Y) {
		r();
		X = this->X;
		Y = this->Y;
	}

	void r() {
		ADC_ChannelConfTypeDef sConfig;
		
		// measure Y
		pinMode(TOUCH_YP_Pin, GPIO_MODE_ANALOG);
		pinMode(TOUCH_YM_Pin, GPIO_MODE_ANALOG);
		pinMode(TOUCH_XP_Pin, GPIO_MODE_OUTPUT_PP);
		pinMode(TOUCH_XM_Pin, GPIO_MODE_OUTPUT_PP);

		sConfig.Channel = ADC_CHANNEL_0;
		sConfig.Rank = 1;
		sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
		configASSERT(HAL_ADC_ConfigChannel(&hadc1, &sConfig) == HAL_OK);

		digitalWrite(TOUCH_XP_Pin, GPIO_PIN_SET);
		digitalWrite(TOUCH_XM_Pin, GPIO_PIN_RESET);

		Y = measure();

		// ================== X AXIS ================
		pinMode(TOUCH_XP_Pin, GPIO_MODE_ANALOG);
		pinMode(TOUCH_XM_Pin, GPIO_MODE_ANALOG);
		pinMode(TOUCH_YP_Pin, GPIO_MODE_OUTPUT_PP);
		pinMode(TOUCH_YM_Pin, GPIO_MODE_OUTPUT_PP);

		sConfig.Channel = ADC_CHANNEL_1;
		sConfig.Rank = 1;
		sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
		configASSERT(HAL_ADC_ConfigChannel(&hadc1, &sConfig) == HAL_OK);

		digitalWrite(TOUCH_YP_Pin, GPIO_PIN_SET);
		digitalWrite(TOUCH_YM_Pin, GPIO_PIN_RESET);

		X = measure();


		// Set X+ to ground
		pinMode(TOUCH_XP_Pin, GPIO_MODE_OUTPUT_PP);
		digitalWrite(TOUCH_XP_Pin, GPIO_PIN_RESET);

		// Set Y- to VCC
		pinMode(TOUCH_YM_Pin, GPIO_MODE_OUTPUT_PP);
		digitalWrite(TOUCH_YM_Pin, GPIO_PIN_SET);

		// Hi-Z X- and Y+
		digitalWrite(TOUCH_XM_Pin, GPIO_PIN_RESET);
		pinMode(TOUCH_XM_Pin, GPIO_MODE_ANALOG);
		digitalWrite(TOUCH_YP_Pin, GPIO_PIN_RESET);
		pinMode(TOUCH_YP_Pin, GPIO_MODE_ANALOG);
	}

	void pinMode(int pin, int mode) {
		GPIO_InitTypeDef GPIO_InitStruct;
		GPIO_InitStruct.Pin = pin;
		GPIO_InitStruct.Mode = mode;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	}

	void digitalWrite(int pin, GPIO_PinState state) {
		HAL_GPIO_WritePin(GPIOA, pin, state);
	}

	int adcRead() {
		configASSERT(HAL_ADC_Start(&hadc1) == HAL_OK);
		configASSERT(HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) == HAL_OK);

		int val = HAL_ADC_GetValue(&hadc1);
		HAL_ADC_Stop(&hadc1);
		return val;
	}

	int measure() {
		for(int i = 0; i < SAMPLES_NUM; i++) {
			samples[i] = adcRead();
		}
		for(int i = 0; i < SAMPLES_NUM; i++) {
			for(int j = 0; j < SAMPLES_NUM - 1; j++) {
				if(samples[j] > samples[j + 1]) {
					int tmp = samples[j];
					samples[j] = samples[j + 1];
					samples[j + 1] = tmp;
				}
			}
		}

		return samples[SAMPLES_NUM / 2];
	}

private:
	int samples[SAMPLES_NUM];

	int X, Y;
};