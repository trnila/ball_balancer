#pragma once
#include "config.h"
#include "configuration.h"

struct Measurement {
	float cx, cy;     // normal change
	float vx, vy;     // current velocity (deprecated)
	float posx, posy; // current positon
	float rvx, rvy;   // current velocity
	float rax, ray;   // current acceleration
	float nx, ny;     // current normal
	float RX, RY;     // filtered resistance
	float USX, USY;   // servo usec
	float rawx, rawy; // measured position
};