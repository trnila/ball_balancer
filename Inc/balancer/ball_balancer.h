#pragma once
#include "config.h"
#include "velocity_tracker.h"
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

class BallBalancer {
public:
	BallBalancer(ITouch *touch, Configuration &conf) noexcept:
			touch(touch),
			conf(conf),
			target(SIZE_X / 2, SIZE_Y / 2)
	{
		reset();
	}

	void reset() {
		planeNormal = Vector3<double>(0, 0, 1);
	}

	bool update(Measurement &meas);

	void setTargetPosition(int x, int y) {
		target = Vectorf(x, y);
	}

	const Vectorf getTargetPosition() const {
		return target;
	}

private:
	ITouch *touch;
	VelocityTracker tracker;
	Configuration &conf;

	Vector3<double> planeNormal;

	Vectorf target;
};
