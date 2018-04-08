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

xQueueHandle rx_queue;
xQueueHandle tx_queue;

buffer_pool<Frame> tx(8), rx(8);

volatile int transmitting = 0;
Frame *current_rx_frame = nullptr;
Frame *current_tx_frame = nullptr;

// temporary buffer used for building frame
char prepare_buffer[MAX_BUF];
// buffer for decoded message
char decoded_buffer[MAX_BUF];

void send_command(uint8_t cmd, char *data, int size) {
	const int HEADER_SIZE = 1;

	taskENTER_CRITICAL();
	Frame *buffer = tx.borrow();
	taskEXIT_CRITICAL();
	configASSERT(buffer);
	configASSERT(size + HEADER_SIZE < MAX_BUF);

	// create frame
	prepare_buffer[0] = cmd;
	memcpy(prepare_buffer + 1, data, size);

	// encode frame
	buffer->size = stuff_data((uint8_t*) prepare_buffer, size + HEADER_SIZE, (uint8_t*) buffer->buffer);

	// add terminator
	buffer->buffer[buffer->size] = '\0';
	buffer->size++;

	configASSERT(xQueueSend(tx_queue, &buffer, portMAX_DELAY) == pdPASS);
	xTaskNotify(uartHandle, TX_BIT, eSetBits);
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if(current_rx_frame->buffer[current_rx_frame->size] == 0) {
		xQueueSendFromISR(rx_queue, &current_rx_frame, NULL);

		current_rx_frame = rx.borrow();
		configASSERT(current_rx_frame);
		current_rx_frame->size = 0;

		xTaskNotifyFromISR(uartHandle, RX_BIT, eSetBits, &xHigherPriorityTaskWoken);
	} else {
		current_rx_frame->size++;
		if(current_rx_frame->size >= MAX_BUF) {
			current_rx_frame->size = 0;
		}
	}

	configASSERT(HAL_UART_Receive_IT(&huart1, (uint8_t*) current_rx_frame->buffer + current_rx_frame->size, 1) == HAL_OK);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	transmitting = 0;
	xTaskNotifyFromISR(uartHandle, TX_BIT, eSetBits, &xHigherPriorityTaskWoken);
}

extern "C" void uart_init() {
	current_rx_frame = rx.borrow();
	current_rx_frame->size = 0;

	configASSERT(rx_queue = xQueueCreate(5, sizeof(Frame*)));
	configASSERT(tx_queue = xQueueCreate(5, sizeof(Frame*)));
	configASSERT(HAL_UART_Receive_IT(&huart1, (uint8_t*) current_rx_frame->buffer, 1) == HAL_OK);
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
				if(current_tx_frame) {
					taskENTER_CRITICAL();
					tx.give(current_tx_frame);
					taskEXIT_CRITICAL();
				}

				configASSERT(frame);
				configASSERT(frame->size > 0);
				configASSERT(frame->size < MAX_BUF);
				current_tx_frame = frame;

				transmitting = 1;
				configASSERT(HAL_UART_Transmit_DMA(&huart1, (uint8_t *) frame->buffer, frame->size) == HAL_OK);
			}
		}

		if((notifiedValue & RX_BIT) != 0) {
			Frame *frame;
			while(xQueueReceive(rx_queue, &frame, 0) == pdTRUE) {
				unstuff_data(reinterpret_cast<const uint8_t *>(frame->buffer), frame->size, reinterpret_cast<uint8_t *>(decoded_buffer));

				uint8_t cmd = *decoded_buffer;
				// align data, otherwise conversion from double will crash
				memmove(decoded_buffer, decoded_buffer + 1, frame->size);

				processCommand(cmd, decoded_buffer);

				portDISABLE_INTERRUPTS();
				rx.give(frame);
				portENABLE_INTERRUPTS();
			}
		}
	}
}