#pragma once

template<typename T>
class buffer_pool {
public:
	buffer_pool(int size): size(size) {
		data = new T[size];
		free = new T*[size];

		for(int i = 0; i < size; i++) {
			free[i] = &data[i];
		}
	}

	~buffer_pool() {
		delete[] data;
	}

	T* borrow() {
		for(int i = 0; i < size; i++) {
			if(free[i]) {
				T* buffer = free[i];
				free[i] = 0;
				return buffer;
			}
		}
		return nullptr;
	}

	void give(T* buffer) {
		for(int i = 0; i < size; i++) {
			if (free[i] == nullptr) {
				free[i] = buffer;
				return;
			}
		}

		configASSERT(0);
	}
private:
	int size;
	T* data;
	T** free;
};