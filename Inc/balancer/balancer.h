#pragma once
#include "config.h"
#include "configuration.h"
#include "vector2.h"

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

void balancer_reset();
Vectorf balancer_current_target();
void balancer_set_target(int x, int y);