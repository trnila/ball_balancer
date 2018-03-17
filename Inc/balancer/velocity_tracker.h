#pragma once

#include <algorithm>
#include <vector>
#include "vector2.h"
#include "vector3.h"
#include "utils.h"
#include "config.h"
#include "itouch.h"
#include "filters.h"

class VelocityTracker {
public:
	VelocityTracker(itouch *touch):
			touch(touch),
			xfilter(new ChainFilter(std::vector<IFilter*>{new VFilter(), new GHFilter(0.1, 0.001), new AvgFilter(2)})),
			yfilter(new ChainFilter(std::vector<IFilter*>{new VFilter(), new GHFilter(0.1, 0.001), new AvgFilter(2)}))
	{}
	bool update() {
		int RX, RY;
		touch->read(RX, RY);

		rawResistance.x = RX;
		rawResistance.y = RY;

		bool touch = !(RX < RminX || RX > RmaxX || RY < RminY || RY > RmaxY);
		if(!touch) {
			return false;
		}

		RX = xfilter->process(RX);
		RY = yfilter->process(RY);


		// store resistance just for debugging purposes
		curResistance.x = RX;
		curResistance.y = RY;

		// map resistance to position in milimetters
		prevPos = curPos;
		curPos.x = map(RX, RminX, RmaxX, 0, SIZE_X);
		curPos.y = map(RY, RminY, RmaxY, 0, SIZE_Y);

		prevSpeed = curSpeed;
		curSpeed = (curPos - prevPos) / (MEASUREMENT_PERIOD_MS / 1000.0);
		acceleration = (curSpeed - prevSpeed) / (MEASUREMENT_PERIOD_MS / 1000.0);

		return true;
	}

	const Vector<int>& getResistance() const {
		return curResistance;
	}

	const Vector<double>& getSpeed() const {
		return curSpeed;
	}

	const Vector<double>& getAcceleration() const {
		return acceleration;
	}

	const Vector<double>& getPos() const {
		return curPos;
	}

	const Vector<int>& getRawResistance() const {
		return rawResistance;
	}

private:
	itouch *touch;
	Vector<int> curResistance;
	Vector<int> rawResistance;
	Vector<double> prevPos, curPos;
	Vector<double> prevSpeed, curSpeed;
	Vector<double> acceleration;

	IFilter *xfilter;
	IFilter *yfilter;
};
