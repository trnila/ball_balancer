#pragma once
#include <algorithm>
#include <assert.h>

template<typename T>
class Samples {
public:
	Samples(int capacity): capacity(capacity), count(0), last(0) {
		samples = new T[capacity];
		sorted = new T[capacity];
	}

	void add(T sample) {
		samples[last] = sample;
		last = (last + 1) % capacity;
		if(count < capacity) {
			count++;
		}
	}

	T median() {
		for(int i = 0; i < count; i++) {
			sorted[i] = samples[i];
		}
		std::sort(sorted, sorted + count);

		return sorted[count / 2];
	}

	bool avg(T& val) {
		if(count <= 0) {
			return false;
		}

		val.x = 0;
		for(int i = 0; i < count; i++) {
			val.x += samples[i].x;
		}
		val.x /= count;

		val.y = 0;
		for(int i = 0; i < count; i++) {
			val.y += samples[i].y;
		}
		val.y /= count;

		return true;
	}

	const T operator[](int index) {
		assert(index < count);
		return samples[index];
	}

	~Samples() {
		delete[] samples;
		delete[] sorted;
	}

private:
	T *samples;
	T *sorted;
	int capacity;
	int count;
	int last;
};
