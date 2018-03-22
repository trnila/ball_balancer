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
void writeServos(int x, int y);
void send(Measurement& measurement);


STMTouch touch;
VelocityTracker tracker(&touch);
Configuration conf;
ball_balancer balancer(tracker, conf);

xQueueHandle txqueue;
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

	configASSERT(txqueue = xQueueCreate(5, sizeof(Measurement)));

	configASSERT(HAL_ADCEx_Calibration_Start(&hadc1) == HAL_OK);

	TickType_t ticks = xTaskGetTickCount();

	Measurement measurement;
	for(;;) {
		HAL_GPIO_WritePin(DEBUG_GPIO_Port, DEBUG_Pin, GPIO_PIN_SET);
		if(balancer.update(measurement)) {
			set_pwm(TIM_CHANNEL_1, measurement.USX);
			set_pwm(TIM_CHANNEL_2, measurement.USY);

			UBaseType_t waiting = uxQueueMessagesWaiting(txqueue);
			if(waiting == 0) {
				txbuffer = measurement;
				configASSERT(HAL_UART_Transmit_DMA(&huart1, (uint8_t *) &txbuffer, sizeof(txbuffer)) == HAL_OK);
			} else {
				xQueueSend(txqueue, &measurement, 0);
			}
		}
		HAL_GPIO_WritePin(DEBUG_GPIO_Port, DEBUG_Pin, GPIO_PIN_RESET);

		handleInput();

		vTaskDelayUntil(&ticks, MEASUREMENT_PERIOD_MS);
	}
}

void set_pwm(uint32_t channel, int us) {
	int pulse = us * 3.9063;

	configASSERT(pulse < 0xFFFF);

	TIM_OC_InitTypeDef conf;
	conf.Pulse = pulse;
	conf.OCMode = TIM_OCMODE_PWM1;
	conf.OCPolarity = TIM_OCPOLARITY_HIGH;
	conf.OCFastMode = TIM_OCFAST_ENABLE;
	configASSERT(HAL_TIM_PWM_ConfigChannel(&htim3, &conf, channel) == HAL_OK);
	configASSERT(HAL_TIM_PWM_Start_IT(&htim3, channel) == HAL_OK);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if(xQueueReceiveFromISR(txqueue, &txbuffer, nullptr) == pdTRUE) {
		configASSERT(HAL_UART_Transmit_DMA(&huart1, (uint8_t *) &txbuffer, sizeof(txbuffer)) == HAL_OK);
	}
}
