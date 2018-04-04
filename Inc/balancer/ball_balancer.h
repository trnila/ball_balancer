#pragma once
#include "config.h"
#include "velocity_tracker.h"
#include "configuration.h"

struct Measurement {
	float cx, cy;
	float vx, vy;
	float posx, posy;
	float rvx, rvy;
	float rax, ray;
	float nx, ny;
	float RX, RY;
	float USX, USY;
	float rawx, rawy;
};

class ball_balancer {
public:
	ball_balancer(VelocityTracker &tracker, Configuration &conf): tracker(tracker), conf(conf), target(SIZE_X / 2, SIZE_Y / 2) {
		reset();
	}

	void reset() {
		planeNormal = Vector3<double>(0, 0, 1);
	}

	bool update(Measurement &meas);

	void setTargetPosition(int x, int y) {
		target = Vectorf(x, y);
	}

private:
	VelocityTracker &tracker;
	Configuration &conf;

	Vector3<double> planeNormal;

	Vectorf target;
};
