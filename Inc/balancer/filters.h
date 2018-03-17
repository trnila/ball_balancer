#pragma once

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