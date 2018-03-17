#pragma once

#include <algorithm>
#include <vector>
#include "Samples.h"
#include "vector2.h"
#include "vector3.h"
#include "Utils.h"
#include "config.h"
#include "ITouch.h"

class Filter: public ITouch {
public:
	Filter(ITouch *touch): fails(0), touch(touch) {}

	void read(int &RX, int &RY) {
		touch->read(RX, RY);

		const int threshold = 3000;
		const int max_set = 65000;

		if(std::abs(RX - prevPos.x) > threshold) {
			RX = prevPos.x + prevSpeed.x;
			fails++;

			if(fails > 20) {
				RX = max_set;
			}
		} else {
			fails = 0;
		}
		prevSpeed.x = RX - prevPos.x;


		if(std::abs(RY - prevPos.y) > threshold) {
			RY = prevPos.y + prevSpeed.y;
			fails++;

			if(fails > 20) {
				RY = max_set;
			}
		} else {
			fails = 0;
		}
		prevSpeed.y = RY - prevPos.y;

		prevPos.x = RX;
		prevPos.y = RY;
	}

private:
	Vector<int> prevPos;
	Vector<int> prevSpeed;
	int fails;
	ITouch *touch;
};

class Filter2: public ITouch {
public:
	Filter2(ITouch *touch): touch(touch), samples(30) {}

	void read(int &RX, int &RY) {
		touch->read(RX, RY);

		Vector<int> sample;
		sample.x = RX;
		sample.y = RY;
		samples.add(sample);

		int diffX, diffY;
		diffX = abs(RX - samples.median().x);
		diffY = abs(RY - samples.median().y);

		const int threshold = 500;
		if(diffX > threshold || diffY > threshold) {
			RX = samples.median().x;
			RY = samples.median().y;
		}
	}

private:
	ITouch *touch;
	Samples< Vector<int> > samples;
};


class IFilter {
public:
	virtual int process(int z) = 0;
	virtual void reset() {}
};

class GHFilter: public IFilter {
public:
	GHFilter(float g, float h): x_est(0), dx(0), h(h), g(g) {}

	virtual int process(int z) {
		float dt = 1;

		float x_pred = x_est + (dx * dt);

		float residual = z - x_pred;
		dx = dx + h * (residual) / dt;

		x_est = x_pred + g * residual;

		return x_est;
	}

	virtual void reset() {
		x_est = 0;
		dx = 0;
	}

private:
	float x_est;
	float dx;

	float h;
	float g;
};

class VFilter: public IFilter {
public:
	VFilter(): prev(0), speed(0) {}

	virtual int process(int z) {
		/*int threshold = 200;

		if(std::abs(z - prev) > threshold) {
			z = prev + speed;
			fails++;

			if(fails > 30) {
				z = 0;
			}
		} else {
			fails = 0;
		}
		speed = z - prev;
		prev = z;*/

		if(z == 0) {
			z = prev + speed;
		} else {
			speed = z - prev;
		}
		prev = z;

		return z;
	};

	virtual void reset() {
		prev = 0;
		speed = 0;
	}

private:
	int prev;
	int speed;
};

class AvgFilter: public IFilter {
public:
	AvgFilter(int size): pos(0), size(size), count(0) {
		samples = new int[size];
	}

	virtual int process(int z) {
		samples[pos++] = z;
		if(pos >= size) {
			pos = 0;
		}
		count++;

		int sum = 0;
		int total = std::min(count, size);
		for(int i = 0; i < total; i++) {
			sum += samples[i];
		}

		return sum / total;
	}

	virtual void reset() {
		count = 0;
		pos = 0;
	}

private:
	int *samples;
	int pos;
	int size;
	int count;
};

class NullFilter: public IFilter {
public:
	virtual int process(int z) {
		return z;
	}
};

class ChainFilter: public IFilter {
public:
	ChainFilter(std::vector<IFilter*> filters): filters(filters) {}

	virtual int process(int z) {
		for(auto &filter: filters) {
			z = filter->process(z);
		}
		return z;
	}

	virtual void reset() {
		for(auto &filter: filters) {
			filter->reset();
		}
	}

private:
	std::vector<IFilter*> filters;
};


class VelocityTracker {
public:
	VelocityTracker(ITouch *touch):
			touch(touch),
			avgSpeed(1),
			xfilter(new ChainFilter(std::vector<IFilter*>{new VFilter(), new GHFilter(0.1, 0.001), new AvgFilter(2)})),
			yfilter(new ChainFilter(std::vector<IFilter*>{new VFilter(), new GHFilter(0.1, 0.001), new AvgFilter(2)}))
			//xfilter(new NullFilter()),
			//yfilter(new NullFilter())
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
	ITouch *touch;
	Vector<int> curResistance;
	Vector<int> rawResistance;
	Vector<double> prevPos, curPos;
	Vector<double> prevSpeed, curSpeed;
	Vector<double> acceleration;
	Samples< Vector<double> > avgSpeed;
	Vector<double> avg;

	IFilter *xfilter;
	IFilter *yfilter;
};
