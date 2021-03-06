#include <math.h>
#include <tim.h>
#include <stm32f1xx_hal_tim.h>
#include <cstring>
#include "stm32f1xx.h"
#include "usart.h"
#include "balancer/balancer.h"
#include "cmsis_os.h"
#include "portmacro.h"
#include "adc.h"
#include "measure.h"
#include "balancer/utils.h"
#include "balancer/vector2.h"
#include "balancer/vector3.h"
#include <ros.h>
#include <std_srvs/SetBool.h>
#include <ballbalancer_msgs/Measurement.h>
#include <ballbalancer_msgs/SetTargetPosition.h>

ros::NodeHandle nh;

ballbalancer_msgs::Measurement measurement;
ros::Publisher measurement_topic("measurements", &measurement);



void set_control(const std_srvs::SetBoolRequest &req, std_srvs::SetBoolResponse &res);
ros::ServiceServer<std_srvs::SetBoolRequest, std_srvs::SetBoolResponse> control_srv("/control", set_control);


void set_target_position(const ballbalancer_msgs::SetTargetPositionRequest &req, ballbalancer_msgs::SetTargetPositionResponse &resp);
ros::ServiceServer<ballbalancer_msgs::SetTargetPositionRequest, ballbalancer_msgs::SetTargetPositionResponse> set_target_pos_srv("SetTargetPosition", set_target_position);


// flush remaining transfers when current transmission completes
extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	nh.getHardware()->flush();
}

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

void fake_measure(int *X, int *Y) {
	const int r = 50/2;

	static int side = 1;
	static int x = -r;

	*X = side * x + target.x;
	*Y = side * (int)sqrt(r*r - x*x) + target.y;

	x++;
	if(x == r) {
		side *= -1;
		x = -r;
	}
}

void calc(ballbalancer_msgs::Measurement &measurement) {
	// get actual reading
	int RX, RY;
	measure_get_current(&RX, &RY);
	//fake_measure(&RX, &RY);

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

	measurement.raw_x = RX;
	measurement.raw_y = RY;
	measurement.pos_x = curPos.x;
	measurement.pos_y = curPos.y;
	measurement.servo_x = USX;
	measurement.servo_y = USY;
	measurement.target_x = target.x;
	measurement.target_y = target.y;
}

void set_control(const std_srvs::SetBoolRequest &req, std_srvs::SetBoolResponse &res) {
	if(req.data) {
		nh.logerror("enabling control");
	} else {
		nh.logerror("disabling control");
	}
}

void set_target_position(const ballbalancer_msgs::SetTargetPositionRequest &req, ballbalancer_msgs::SetTargetPositionResponse &resp) {
	nh.logdebug("Setting new target position");
	balancer_set_target(req.x, req.y);
}



void controlTask(void const * argument) {
	nh.initNode();

	nh.advertise(measurement_topic);

	nh.advertiseService(control_srv);
	nh.advertiseService(set_target_pos_srv);

	// initialize pwm for servos
	configASSERT(HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_1) == HAL_OK);
	configASSERT(HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_2) == HAL_OK);
	configASSERT(HAL_ADCEx_Calibration_Start(&hadc1) == HAL_OK);

	balancer_reset();

	configASSERT(xTaskCreate(measureTask, "measure", 512, NULL, tskIDLE_PRIORITY, NULL) == pdTRUE);

	TickType_t ticks = xTaskGetTickCount();
	for(;;) {
		calc(measurement);
		set_pwm(TIM_CHANNEL_1, measurement.servo_x);
		set_pwm(TIM_CHANNEL_2, measurement.servo_y);

		measurement_topic.publish(&measurement);

		nh.spinOnce();
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