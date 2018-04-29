#include <cmsis_os.h>
#include <algorithm>
#include "balancer/stm_touch.h"
#include "adc.h"

void STMTouch::read(int &X, int &Y) {
	Y = measureY();
	X = measureX();
}

int STMTouch::measureY() {
	pinMode(TOUCH_YP_Pin, GPIO_MODE_INPUT);
	pinMode(TOUCH_YM_Pin, GPIO_MODE_INPUT);
	pinMode(TOUCH_XP_Pin, GPIO_MODE_OUTPUT_PP);
	pinMode(TOUCH_XM_Pin, GPIO_MODE_OUTPUT_PP);

	digitalWrite(TOUCH_XP_Pin, GPIO_PIN_SET);
	digitalWrite(TOUCH_XM_Pin, GPIO_PIN_RESET);

	return measureAxis(ADC_CHANNEL_0);
}

void STMTouch::selectADCChannel(uint32_t channel) const {
	ADC_ChannelConfTypeDef sConfig{};
	sConfig.Channel = channel;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
	configASSERT(HAL_ADC_ConfigChannel(&hadc1, &sConfig) == HAL_OK);
}

int STMTouch::measureX() {
	pinMode(TOUCH_XP_Pin, GPIO_MODE_INPUT);
	pinMode(TOUCH_XM_Pin, GPIO_MODE_INPUT);
	pinMode(TOUCH_YP_Pin, GPIO_MODE_OUTPUT_PP);
	pinMode(TOUCH_YM_Pin, GPIO_MODE_OUTPUT_PP);

	digitalWrite(TOUCH_YP_Pin, GPIO_PIN_SET);
	digitalWrite(TOUCH_YM_Pin, GPIO_PIN_RESET);

	return measureAxis(ADC_CHANNEL_1);
}

void STMTouch::pinMode(int pin, int mode) {
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = pin;
	GPIO_InitStruct.Mode = mode;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void STMTouch::digitalWrite(int pin, GPIO_PinState state) {
	HAL_GPIO_WritePin(GPIOA, pin, state);
}

int STMTouch::measureAxis(uint32_t channel) {
	selectADCChannel(channel);
	osDelay(2); // wait for discharge

	measureSamples();
	std::sort(samples, samples + SAMPLES_NUM);

	uint32_t sum = 0;
	for(int i = SAMPLES_NUM / 2 - SAMPLES_TAKE; i < SAMPLES_NUM / 2 + SAMPLES_TAKE; i++) {
		sum += samples[i];
	}

	return sum / (SAMPLES_TAKE * 2);
}

void STMTouch::measureSamples() {
	configASSERT(HAL_ADC_Start(&hadc1) == HAL_OK);
	for(int i = 0; i < SAMPLES_NUM; i++) {
		configASSERT(HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) == HAL_OK);
		samples[i] = HAL_ADC_GetValue(&hadc1);
	}
	HAL_ADC_Stop(&hadc1);
}
