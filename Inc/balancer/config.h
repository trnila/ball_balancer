#pragma once

#define STM32

const int MX = 85, MY = 85; // montazni bod kloubu na ose x, y
const int PX = 29, PY = 29; // delka paky na servu na ose x, y
const int CENTER_X_US = 1000;
const int CENTER_Y_US = 1000;
const double SHIFT_MM = 14;
const int DUTY_MS = 20;

const int SIZE_Y = 230;
const int SIZE_X = 170;

const int MEASUREMENT_PERIOD_MS = 20;
const int CONTROL_PERIOD_MS = DUTY_MS / MEASUREMENT_PERIOD_MS;

const int MAX_CMD_ARGS = 6;

//const int RminY = 10600, RmaxY = 44900;
//const int RminX = 8130, RmaxX = 50700;

//const int RminY = 10600.0 / 65535 * 4096, RmaxY = 44900.0 / 65535 * 4096;
//const int RminX = 8130.0 / 65535 * 4096, RmaxX = 50700.0 / 65535 * 4096;

const int RminX = 366, RmaxX = 3726;
const int RminY = 378, RmaxY = 3650;