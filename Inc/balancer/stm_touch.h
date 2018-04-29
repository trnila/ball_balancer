#pragma once

#include "balancer/touch.h"

extern "C" {
	#include "stm32f1xx.h"
}

#define SAMPLES_NUM 32
#define SAMPLES_TAKE 4 // number of samples from median

class STMTouch: public ITouch {
public:
	void read(int &X, int &Y) override;


private:
	uint16_t samples[SAMPLES_NUM]{};

	int measureY();
	int measureX();
	int measureAxis(uint32_t channel);
	void measureSamples();

	void selectADCChannel(uint32_t channel) const;
	void pinMode(uint32_t pin, uint32_t mode);
	void digitalWrite(int pin, GPIO_PinState state);
};