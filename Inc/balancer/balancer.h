#pragma once
#include "config.h"
#include "configuration.h"
#include "vector2.h"

void balancer_reset();
Vectorf balancer_current_target();
void balancer_set_target(int x, int y);