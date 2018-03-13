#include <tim.h>
#include <stm32f1xx_hal_tim.h>
#include "stm32f1xx.h"
#include "usart.h"
#include "BallBalancer.h"
#include "STMTouch.h"
#include "cmsis_os.h"
#include "string.h"
//#include <tim.h>

extern "C" {
	void controlTask(void const * argument);
	void inputTask(void const * argument);
};

void set_pwm(uint32_t channel, double val);
void writeServos(double x, double y);
void send(Measurement& measurement);


STMTouch touch;
Filter filter(&touch);
VelocityTracker tracker(&touch);
Configuration conf;
BallBalancer balancer(tracker, conf, writeServos, send);

Measurement txbuffer;

#define MAX_BUF 32
#define MAX_CMD_ARGS 5
char rxbuffer[MAX_BUF];
int rxpos = 0;
xQueueHandle rxcommands;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if(rxbuffer[rxpos] == '\n') {
		rxbuffer[rxpos] = '\0';
		xQueueSendFromISR(rxcommands, rxbuffer, NULL);

		rxpos = 0;
	} else {
		rxpos = (rxpos + 1) % MAX_BUF;
	}

	int x = HAL_UART_Receive_IT(&huart1, (uint8_t*) rxbuffer + rxpos, 1);
	configASSERT(x == HAL_OK);
}

void processCommand(char* cmd, int argc, char** argv) {
	if(strcmp(cmd, "set_p") == 0 && argc >= 1) {
		conf.const_p = atof(argv[0]);
	} else if(strcmp(cmd, "set_d")  == 0 && argc >= 1) {
		conf.const_d = atof(argv[0]);
	} else if(strcmp(cmd, "set_i")  == 0 && argc >= 1) {
		conf.const_i = atof(argv[0]);
	}
}

void handleInput() {
	char buffer[MAX_BUF];
	while(xQueueReceive(rxcommands, &buffer, 0) == pdTRUE) {
		char* args[MAX_CMD_ARGS];

		char *tok = strtok(buffer, " ");
		char *cmd = tok;

		int i = 0;
		while(tok && i < MAX_CMD_ARGS) {
			args[i] = tok = strtok(NULL, " ");
			i++;
		}

		processCommand(cmd, i - 1, args);
	}
}

void controlTask(void const * argument) {
	// initialize pwm for servos
	configASSERT(HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_1) == HAL_OK);
	configASSERT(HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_2) == HAL_OK);

	configASSERT(rxcommands = xQueueCreate(5, MAX_BUF));
	configASSERT(HAL_UART_Receive_IT(&huart1, (uint8_t*) &rxbuffer, 1) == HAL_OK);

	TickType_t ticks = xTaskGetTickCount();

	for(;;) {
		HAL_GPIO_WritePin(DEBUG_GPIO_Port, DEBUG_Pin, GPIO_PIN_SET);
		balancer.update();
		HAL_GPIO_WritePin(DEBUG_GPIO_Port, DEBUG_Pin, GPIO_PIN_RESET);

		handleInput();

		vTaskDelayUntil(&ticks, MEASUREMENT_PERIOD_MS);
	}
}

void set_pwm(uint32_t channel, double val) {
	TIM_OC_InitTypeDef conf;
	conf.Pulse = val * htim3.Init.Period;
	conf.OCMode = TIM_OCMODE_PWM1;
	conf.OCPolarity = TIM_OCPOLARITY_HIGH;
	conf.OCFastMode = TIM_OCFAST_ENABLE;
	configASSERT(HAL_TIM_PWM_ConfigChannel(&htim3, &conf, channel) == HAL_OK);
	configASSERT(HAL_TIM_PWM_Start_IT(&htim3, channel) == HAL_OK);
}

void writeServos(double x, double y) {
	set_pwm(TIM_CHANNEL_1, x);
	set_pwm(TIM_CHANNEL_2, y);
}

void send(Measurement &measurement) {
	txbuffer = measurement;
	HAL_UART_Transmit_IT(&huart1, (uint8_t*) &txbuffer, sizeof(txbuffer));
}
