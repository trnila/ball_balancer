#include <math.h>
#include <tim.h>
#include <stm32f1xx_hal_tim.h>
#include <cstring>
#include "stm32f1xx.h"
#include "usart.h"
#include "balancer/ball_balancer.h"
#include "cmsis_os.h"
#include "portmacro.h"
#include "balancer/comm.h"
#include "adc.h"
#include "balancer/benchmark.h"
#include "measure.h"
#include "balancer/utils.h"
#include "balancer/vector2.h"
#include "balancer/vector3.h"


extern "C" void controlTask(void const * argument);
void set_pwm(uint32_t channel, int us);

Configuration conf;
Vectorf prevPos, curPos, prevSpeed, curSpeed;
Vectorf target = {SIZE_X / 2, SIZE_Y / 2};
Vector3<double> planeNormal = Vector3<double>(0, 0, 1);

void calc(Measurement &measurement) {
	// get actual reading
	int RX, RY;
	portENTER_CRITICAL();
	RX = X;
	RY = Y;
	portEXIT_CRITICAL();

	// calculate pos, speeds
	bool touch = !(RX < RminX || RX > RmaxX || RY < RminY || RY > RmaxY);
	if(!touch) {
		return;
	}

	// map resistance to position in milimetters
	prevPos = curPos;
	curPos.x = map(RX, RminX, RmaxX, 0, SIZE_X);
	curPos.y = map(RY, RminY, RmaxY, 0, SIZE_Y);

	prevSpeed = curSpeed;
	curSpeed = (curPos - prevPos) / (MEASUREMENT_PERIOD_MS / 1000.0);
	Vectorf acceleration = (curSpeed - prevSpeed) / (MEASUREMENT_PERIOD_MS / 1000.0);


	// calc pid
	Vectorf change = conf.const_p * curSpeed + conf.const_d * acceleration + conf.const_i * (curPos - target);


	planeNormal.x = cap(planeNormal.x - change.x);
	planeNormal.y = cap(planeNormal.y - change.y);
	planeNormal = normalize(planeNormal);

	double zx = planeNormal.x * MX / planeNormal.z;
	double zy = planeNormal.y * MY / planeNormal.z;

	double angleX = zx / PX;
	double angleY = zy / PY;

	int USX = CENTER_X_US + angleX / 0.5f * 600;
	int USY = CENTER_Y_US + angleY / 0.5f * 600;

	measurement.RX = RX;
	measurement.RY = RY;
	measurement.posx = curPos.x;
	measurement.posy = curPos.y;
	measurement.USX = USX;
	measurement.USY = USY;
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

		calc(measurement);
		set_pwm(TIM_CHANNEL_1, measurement.USX);
		set_pwm(TIM_CHANNEL_2, measurement.USY);

		/*char buffer[40];
		snprintf(buffer, sizeof(buffer), "%d,%d\r\n",
		         (int) measurement.RX, (int) measurement.RY
		);
		HAL_UART_Transmit(&huart1, (uint8_t *) buffer, strlen(buffer), HAL_MAX_DELAY);*/

		sendCommand(CMD_MEASUREMENT, (char *) &measurement, sizeof(measurement));
		benchmark_stop(0, "measure & control cycle");

		vTaskDelayUntil(&ticks, MEASUREMENT_PERIOD_MS);
	}
}


void set_pwm(uint32_t channel, int us) {
	double t = 1.0 / ((double) HAL_RCC_GetHCLKFreq() / (htim3.Init.Prescaler + 1));
	int pulse = (double) us * (pow(10, -6)) / t;

	configASSERT(pulse < 0xFFFF);
	__HAL_TIM_SET_COMPARE(&htim3, channel, pulse);
}