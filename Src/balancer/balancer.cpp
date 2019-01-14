#include <math.h>
#include <tim.h>
#include <stm32f1xx_hal_tim.h>
#include <cstring>
#include "stm32f1xx.h"
#include "usart.h"
#include "balancer/balancer.h"
#include "cmsis_os.h"
#include "portmacro.h"
#include "balancer/comm.h"
#include "adc.h"
#include "balancer/benchmark.h"
#include "measure.h"
#include "balancer/utils.h"
#include "balancer/vector2.h"
#include "balancer/vector3.h"
#include <ros.h>
#include <std_msgs/String.h>

ros::NodeHandle nh;
std_msgs::String str_msg;
ros::Publisher chatter("chatter", &str_msg);

char hello[25];


extern "C" void controlTask(void const * argument);
void set_pwm(uint32_t channel, int us);

Configuration conf;
Vectorf prevPos, curPos, prevSpeed, curSpeed;
Vectorf target = {SIZE_X / 2, SIZE_Y / 2};
Vector3<double> planeNormal;

void limit_vector(Vectorf &v, int limit) {
	if(v.x < -limit) {
		v.x = -limit;
	} else if(v.x > limit) {
		v.x = limit;
	}

	if(v.y < -limit) {
		v.y = -limit;
	} else if(v.y > limit) {
		v.y = limit;
	}
}

void calc(Measurement &measurement) {
	// get actual reading
	int RX, RY;
	measure_get_current(&RX, &RY);

	// calculate pos, speeds
	bool touch = !(RX < RminX || RX > RmaxX || RY < RminY || RY > RmaxY);
	if(!touch) {
		balancer_reset();
		prevPos = curPos = target;
	} else {
		// map resistance to position in milimetters
		prevPos = curPos;
		curPos.x = map(RX, RminX, RmaxX, 0, SIZE_X);
		curPos.y = map(RY, RminY, RmaxY, 0, SIZE_Y);
	}

	prevSpeed = curSpeed;
	curSpeed = (curPos - prevPos) / (MEASUREMENT_PERIOD_MS / 1000.0);
	Vectorf acceleration = (curSpeed - prevSpeed) / (MEASUREMENT_PERIOD_MS / 1000.0);

	limit_vector(curSpeed, 4000);
	limit_vector(acceleration, 4000);


	// calc pid
	Vectorf change = conf.const_p * curSpeed + conf.const_d * acceleration + conf.const_i * (curPos - target);


	planeNormal.x = cap(planeNormal.x - change.x);
	planeNormal.y = cap(planeNormal.y - change.y);
	planeNormal = normalize(planeNormal);

	double zx = -planeNormal.x * MX / planeNormal.z;
	double zy = -planeNormal.y * MY / planeNormal.z;

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
	nh.initNode();
	nh.advertise(chatter);


	int i = 0;
	for(;;) {
		str_msg.data = hello;
		sprintf(hello, "hello %d", i++);

		chatter.publish(&str_msg);
		nh.spinOnce();

		int val;
		if(nh.getParam("/serial_node/baud", &val)) {
			nh.spinOnce();
		}

		vTaskDelay(10);
	}

	for(;;);

	// initialize pwm for servos
	configASSERT(HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_1) == HAL_OK);
	configASSERT(HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_2) == HAL_OK);
	configASSERT(HAL_ADCEx_Calibration_Start(&hadc1) == HAL_OK);

	balancer_reset();

	configASSERT(xTaskCreate(measureTask, "measure", 512, NULL, tskIDLE_PRIORITY, NULL) == pdTRUE);

	TickType_t ticks = xTaskGetTickCount();
	benchmark_init();
	for(;;) {
		benchmark_start(0);

		Measurement measurement{};
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
	int pulse = HAL_RCC_GetHCLKFreq() / 1000000 * us / (htim3.Init.Prescaler + 1);

	configASSERT(pulse < 0xFFFF);
	__HAL_TIM_SET_COMPARE(&htim3, channel, pulse);
}

void balancer_reset() {
	planeNormal = Vector3<double>(0, 0, 1);
}

Vectorf balancer_current_target() {
	// TODO: fix concurency access
	return target;
}

void balancer_set_target(int x, int y) {
	// TODO: fix concurency access
	target.x = x;
	target.y = y;
}