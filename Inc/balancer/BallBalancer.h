#pragma once
#include "config.h"
#include "TouchPanel.h"
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

class BallBalancer {
public:
	BallBalancer(
			VelocityTracker &tracker,
			Configuration &conf,
			void (*writeServos)(int, int),
			void (*send)(Measurement& measurement)
	): tracker(tracker), conf(conf), writeServos(writeServos), send(send), controlPeriodMs(CONTROL_PERIOD_MS) {
		reset();
	}

	void reset() {
		planeNormal = Vector3<double>(0, 0, 1);
	}

	void update() {
		bool updated = tracker.update();

		Vectorf target(SIZE_X / 2, SIZE_Y / 2);
		Vectorf speed = tracker.getSpeed();
		Vectorf accel = tracker.getAcceleration();
		Vectorf change = conf.const_p * speed + conf.const_d * accel + conf.const_i * (tracker.getPos() - target);

		if(updated) {
			planeNormal.x = cap(planeNormal.x - change.x);
			planeNormal.y = cap(planeNormal.y - change.y);
			planeNormal = normalize(planeNormal);
		}

		if(controlPeriodMs == 0 || (controlPeriodMs && i % controlPeriodMs == 0)) {
			double zx = planeNormal.x * MX / planeNormal.z;
			double zy = planeNormal.y * MY / planeNormal.z;

			double angleX = zx / PX;
			double angleY = zy / PY;

			double DX = 10 * angleX * 180 / M_PI; // 1 degree per 10 microseconds
			double DY = 10 * angleY * 180  / M_PI;

			int USX = CENTER_X_US + DX;
			int USY = CENTER_Y_US + DY;
			writeServos(USX, USY);


			Measurement meas;
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
			send(meas);
		}
		i++;
	}

	void setControlPeriod(int ms) {
		controlPeriodMs = ms;
	}

private:
	int i;
	int controlPeriodMs;

	VelocityTracker &tracker;
	Configuration &conf;
	void (*writeServos)(int, int);
	void (*send)(Measurement& measurement);

	Vector3<double> planeNormal;
};
