#pragma once
#include "config.h"
#include "velocity_tracker.h"
#include "configuration.h"

struct __attribute__((__packed__))  Measurement {
	char magic[2];
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

	bool update(Measurement &meas) {
		bool touchDetected = tracker.update();

		Vectorf speed = tracker.getSpeed();
		Vectorf accel = tracker.getAcceleration();
		Vectorf change = conf.const_p * speed + conf.const_d * accel + conf.const_i * (tracker.getPos() - target);

		if(touchDetected) {
			planeNormal.x = cap(planeNormal.x - change.x);
			planeNormal.y = cap(planeNormal.y - change.y);
			planeNormal = normalize(planeNormal);
		}

		double zx = -planeNormal.x * MX / planeNormal.z;
		double zy = -planeNormal.y * MY / planeNormal.z;

		double angleX = zx / PX;
		double angleY = zy / PY;

		int USX = CENTER_X_US + angleX / 0.5f * 600;
		int USY = CENTER_Y_US + angleY / 0.5f * 600;

		meas.magic[0] = 0xAB;
		meas.magic[1] = 0xCD;
		meas.cx = change.x; meas.cy = change.y;
		meas.vx = speed.x; meas.vy = speed.y;
		meas.posx = (float) tracker.getPos().x; meas.posy = (float) tracker.getPos().y;
		meas.rvx = tracker.getSpeed().x; meas.rvy = tracker.getSpeed().y;
		meas.rax = tracker.getAcceleration().x; meas.ray = tracker.getAcceleration().y;
		meas.nx = planeNormal.x; meas.ny = planeNormal.y;
		meas.RX = (float) tracker.getResistance().x; meas.RY = (float) tracker.getResistance().y;
		meas.USX = USX; meas.USY = USY;
		meas.rawx = tracker.getRawResistance().x;
		meas.rawy = tracker.getRawResistance().y;

		return touchDetected;
	}

private:
	VelocityTracker &tracker;
	Configuration &conf;

	Vector3<double> planeNormal;

	Vectorf target;
};
