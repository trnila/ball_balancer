#pragma once
#include <climits>

#define CLOCK_MHZ 64
#define RESOLUTION_US 10
#define MAX_TIMERS 8

void benchmark_init();
void benchmark_start(int id);
void benchmark_stop(int id, const char *msg);