extern "C" {
	#include <FreeRTOSConfig.h>
	#include <FreeRTOS.h>
	#include <queue.h>
	#include <string.h>
	#include <usart.h>
	#include <task.h>
}
#include "balancer/ball_balancer.h"
#include "buffer.h"
#include "comm.h"
#include <climits>
#include "cmsis_os.h"
#include "uart_encoder.h"

#define TX_BIT    0x01
#define RX_BIT    0x02

#define MAX_BUF 128

extern "C" {
	void uartTask(void const * argument);
	extern osThreadId uartHandle;
	extern BaseType_t xHigherPriorityTaskWoken;
}

extern Configuration conf;
extern ball_balancer balancer;


struct Frame {
	char buffer[MAX_BUF];
	int size;
};

xQueueHandle rxcommands;
xQueueHandle tx_queue;

buffer_pool<Frame> tx(8);

Frame currentFrame;
Frame *lastTxFrame = nullptr;
char prepareBuffer[MAX_BUF];
volatile int transmitting = 0;

void send_command(uint8_t cmd, char *data, int size) {
	const int HEADER_SIZE = 1;

	taskENTER_CRITICAL();
	Frame *buffer = tx.borrow();
	taskEXIT_CRITICAL();
	configASSERT(buffer);
	configASSERT(size + HEADER_SIZE < MAX_BUF);

	// create frame
	prepareBuffer[0] = cmd;
	memcpy(prepareBuffer + 1, data, size);

	// encode frame
	buffer->size = stuff_data((uint8_t*) prepareBuffer, size + HEADER_SIZE, (uint8_t*) buffer->buffer);

	// add terminator
	buffer->buffer[buffer->size] = '\0';
	buffer->size++;

	configASSERT(xQueueSend(tx_queue, &buffer, portMAX_DELAY) == pdPASS);
	xTaskNotify(uartHandle, TX_BIT, eSetBits);
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if(currentFrame.buffer[currentFrame.size] == 0) {
		xQueueSendFromISR(rxcommands, &currentFrame, NULL);
		currentFrame.size = 0;

		xTaskNotifyFromISR(uartHandle, RX_BIT, eSetBits, &xHigherPriorityTaskWoken);
	} else {
		currentFrame.size++;
		if(currentFrame.size >= MAX_BUF) {
			currentFrame.size = 0;
		}
	}

	configASSERT(HAL_UART_Receive_IT(&huart1, (uint8_t*) currentFrame.buffer + currentFrame.size, 1) == HAL_OK);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	transmitting = 0;
	xTaskNotifyFromISR(uartHandle, TX_BIT, eSetBits, &xHigherPriorityTaskWoken);
}

extern "C" void uart_init() {
	configASSERT(rxcommands = xQueueCreate(5, sizeof(currentFrame)));
	configASSERT(tx_queue = xQueueCreate(5, sizeof(Frame*)));
	configASSERT(HAL_UART_Receive_IT(&huart1, (uint8_t*) &currentFrame.buffer, 1) == HAL_OK);
}

// FIXME: crashes when on stack? even when aligned
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
	} else if(cmd == CMD_GETPOS) {
		int result[2];
		taskENTER_CRITICAL();
		result[0] = (int) balancer.getTargetPosition().x;
		result[1] = (int) balancer.getTargetPosition().y;
		taskEXIT_CRITICAL();

		send_command(CMD_GETPOS | CMD_RESPONSE, (char*) &result, sizeof(result));
	} else if(cmd == CMD_GETPID) {
		double r[3];
		taskENTER_CRITICAL();
		r[0] = conf.const_p;
		r[1] = conf.const_i;
		r[2] = conf.const_d;
		taskEXIT_CRITICAL();

		send_command(CMD_GETPID | CMD_RESPONSE, (char*) &r, sizeof(r));
	} else if(cmd == CMD_GETDIM) {
		int result[] = {SIZE_X, SIZE_Y};
		send_command(CMD_GETDIM | CMD_RESPONSE, (char*) &result, sizeof(result));
	} else {
		//configASSERT(0);
	}
}

void uartTask(void const * argument) {
	for(;;) {
		uint32_t notifiedValue;
		configASSERT(xTaskNotifyWait( pdFALSE, RX_BIT, &notifiedValue, portMAX_DELAY) == pdPASS);

		if(transmitting == 0 && (notifiedValue & TX_BIT) != 0) {
			Frame *frame = nullptr;
			if(xQueueReceive(tx_queue, &frame, 0) == pdTRUE) {
				if(lastTxFrame) {
					taskENTER_CRITICAL();
					tx.give(lastTxFrame);
					taskEXIT_CRITICAL();
				}

				configASSERT(frame);
				configASSERT(frame->size > 0);
				configASSERT(frame->size < MAX_BUF);
				lastTxFrame = frame;

				transmitting = 1;
				configASSERT(HAL_UART_Transmit_DMA(&huart1, (uint8_t *) frame->buffer, frame->size) == HAL_OK);
			}
		}

		if((notifiedValue & RX_BIT) != 0) {
			Frame frame{};
			while(xQueueReceive(rxcommands, &frame, 0) == pdTRUE) {
				char decoded[MAX_BUF];

				unstuff_data(reinterpret_cast<const uint8_t *>(frame.buffer), frame.size, reinterpret_cast<uint8_t *>(decoded));

				uint8_t cmd = *decoded;
				// align data, otherwise conversion from double will crash
				memmove(decoded, decoded + 1, frame.size);

				processCommand(cmd, decoded);
			}
		}
	}
}