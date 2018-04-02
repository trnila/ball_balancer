extern "C" {
	#include <FreeRTOSConfig.h>
	#include <FreeRTOS.h>
	#include <queue.h>
	#include <string.h>
	#include <usart.h>
	#include <task.h>
}
#include "balancer/ball_balancer.h"

extern "C" {
	void uartTask(void const * argument);
}

const uint8_t CMD_RESET = 0;
const uint8_t CMD_POS = 1;
const uint8_t CMD_PID = 2;
const uint8_t CMD_RESPONSE = 128;

Measurement txbuffer;

#define MAX_BUF 32
xQueueHandle rxcommands;

extern Configuration conf;
extern ball_balancer balancer;

struct Frame {
	char buffer[MAX_BUF];
	int size;
};

Frame currentFrame;

size_t UnStuffData(const uint8_t *ptr, size_t length, uint8_t *dst) {
	const uint8_t *start = dst, *end = ptr + length;
	uint8_t code = 0xFF, copy = 0;

	for (; ptr < end; copy--) {
		if (copy != 0) {
			*dst++ = *ptr++;
		} else {
			if (code != 0xFF)
				*dst++ = 0;
			copy = code = *ptr++;
			if (code == 0)
				break; /* Source length too long */
		}
	}
	return dst - start;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if(currentFrame.buffer[currentFrame.size] == 0) {
		xQueueSendFromISR(rxcommands, &currentFrame, NULL);
		currentFrame.size = 0;
	} else {
		currentFrame.size++;
		if(currentFrame.size >= MAX_BUF) {
			currentFrame.size = 0;
		}
	}

	configASSERT(HAL_UART_Receive_IT(&huart1, (uint8_t*) currentFrame.buffer + currentFrame.size, 1) == HAL_OK);
}

void processCommand(uint8_t cmd, char* args) {
	if(cmd == CMD_RESET) {
		balancer.reset();
	} else if(cmd == CMD_POS) {
		int x = *(int*) args;
		int y = *(int*) (args + sizeof(int));

		taskENTER_CRITICAL();
		balancer.setTargetPosition(x, y);
		taskEXIT_CRITICAL();
	} else if(cmd == CMD_PID) {
		double p = *(double*) args;
		double i = *(double*) (args + sizeof(double));
		double d = *(double*) (args + sizeof(double) * 2);

		taskENTER_CRITICAL();
		conf.const_p = p;
		conf.const_i = i;
		conf.const_d = d;
		taskEXIT_CRITICAL();
	} else {

	}
}

void uartTask(void const * argument) {
	configASSERT(rxcommands = xQueueCreate(5, sizeof(currentFrame)));
	configASSERT(HAL_UART_Receive_IT(&huart1, (uint8_t*) &currentFrame.buffer, 1) == HAL_OK);

	for(;;) {
		Frame frame{};
		configASSERT(xQueueReceive(rxcommands, &frame, portMAX_DELAY) == pdTRUE);
		char decoded[MAX_BUF];

		UnStuffData(reinterpret_cast<const uint8_t *>(frame.buffer), frame.size, reinterpret_cast<uint8_t *>(decoded));

		uint8_t cmd = *decoded;
		// align data, otherwise conversion from double will crash
		memmove(decoded, decoded + 1, frame.size);

		processCommand(cmd, decoded);
	}
}