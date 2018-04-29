#pragma once

#include <algorithm>
#include <vector>
#include "vector2.h"
#include "vector3.h"
#include "touch.h"
#include "filters.h"

class VelocityTracker {
public:
	explicit VelocityTracker(ITouch *touch) noexcept:
			touch(touch),
			xfilter(new ChainFilter(std::vector<IFilter*>{new VFilter(150), new GHFilter(0.1, 0.001), new AvgFilter(2)})),
			yfilter(new ChainFilter(std::vector<IFilter*>{new VFilter(150), new GHFilter(0.1, 0.001), new AvgFilter(2)}))
	{}

	bool update();

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
	ITouch *touch;
	Vector<int> curResistance;
	Vector<int> rawResistance;
	Vector<double> prevPos, curPos;
	Vector<double> prevSpeed, curSpeed;
	Vector<double> acceleration;

	IFilter *xfilter;
	IFilter *yfilter;
};
