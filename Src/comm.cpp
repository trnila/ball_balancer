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

extern "C" {
	void uartTask(void const * argument);
	extern osThreadId uartHandle;
}

#define TX_BIT    0x01
#define RX_BIT    0x02

extern BaseType_t xHigherPriorityTaskWoken;

size_t StuffData(const uint8_t *ptr, size_t length, uint8_t *dst);

#define MAX_BUF 128

struct Frame {
	char buffer[MAX_BUF];
	int size;
};

extern buffer_pool<Frame> tx;

xQueueHandle rxcommands;
xQueueHandle tx_queue;

extern Configuration conf;
extern ball_balancer balancer;

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
	buffer->size = StuffData((uint8_t*) prepareBuffer, size + HEADER_SIZE, (uint8_t*) buffer->buffer);

	// add terminator
	buffer->buffer[buffer->size] = '\0';
	buffer->size++;

	configASSERT(xQueueSend(tx_queue, &buffer, portMAX_DELAY) == pdPASS);
	xTaskNotify(uartHandle, TX_BIT, eSetBits);
}

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

#define StartBlock()	(code_ptr = dst++, code = 1)
#define FinishBlock()	(*code_ptr = code)

size_t StuffData(const uint8_t *ptr, size_t length, uint8_t *dst)
{
	const uint8_t *start = dst, *end = ptr + length;
	uint8_t code, *code_ptr; /* Where to insert the leading count */

	StartBlock();
	while (ptr < end) {
		if (code != 0xFF) {
			uint8_t c = *ptr++;
			if (c != 0) {
				*dst++ = c;
				code++;
				continue;
			}
		}
		FinishBlock();
		StartBlock();
	}
	FinishBlock();
	return dst - start;
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

// FIXME: crashes when on stack? even when aligned
double r[3];
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
		configASSERT(0);
	}
}

extern "C" void uart_init() {
	configASSERT(rxcommands = xQueueCreate(5, sizeof(currentFrame)));
	configASSERT(tx_queue = xQueueCreate(5, sizeof(Frame*)));
	configASSERT(HAL_UART_Receive_IT(&huart1, (uint8_t*) &currentFrame.buffer, 1) == HAL_OK);
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

				UnStuffData(reinterpret_cast<const uint8_t *>(frame.buffer), frame.size, reinterpret_cast<uint8_t *>(decoded));

				uint8_t cmd = *decoded;
				// align data, otherwise conversion from double will crash
				memmove(decoded, decoded + 1, frame.size);

				processCommand(cmd, decoded);
			}
		}
	}
}