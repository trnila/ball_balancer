#pragma once
#include "balancer/buffer.h"

const uint8_t CMD_RESPONSE = 128;
const uint8_t CMD_GETTER = 64;

const uint8_t CMD_RESET = 0;
const uint8_t CMD_SET_TARGET = 1;
const uint8_t CMD_PID = 2;

const uint8_t CMD_GET_TARGET = CMD_GETTER | CMD_SET_TARGET;
const uint8_t CMD_GETPID = CMD_GETTER | CMD_PID;
const uint8_t CMD_GETDIM = CMD_GETTER | (CMD_PID + 1);

const uint8_t CMD_MEASUREMENT = 0 | CMD_RESPONSE;
const uint8_t CMD_ERROR_RESPONSE = 255;

void send_command(uint8_t cmd, char* data, int size);