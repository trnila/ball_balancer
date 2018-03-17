#pragma once
#include <cmath>
#include "config.h"

double cap(double val) {
	double limit = (double) SHIFT_MM / MX;
	return std::min(std::max(val, -limit), limit);
}

double map(double val, double a, double b, double c, double d) {
	if(val < a) {
		return c;
	}

	if(val > b) {
		return d;
	}

	return (val - a) / (b - a) * (d - c) + c;
}
