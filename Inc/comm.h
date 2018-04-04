#pragma once
#include "buffer.h"

const uint8_t CMD_RESET = 0;
const uint8_t CMD_POS = 1;
const uint8_t CMD_PID = 2;
const uint8_t CMD_RESPONSE = 128;

const uint8_t CMD_MEASUREMENT = 0 | CMD_RESPONSE;

void send_command(uint8_t cmd, char* data, int size);