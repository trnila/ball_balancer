#include "gtest/gtest.h"

template<typename T, int size>
class Queue {
public:
	Queue(): head(0), tail(0) {}

	bool add(T item) {
		if(full()) {
			return false;
		}

		items[tail] = item;
		tail = inc(tail);
		return true;
	}

	bool get(T* item) {
		if(empty()) {
			return false;
		}

		*item = items[head];
		head = inc(head);
		return true;
	}

	bool empty() {
		return head == tail;
	}

	bool full() {
		return inc(tail) == head;
	}

private:
	int head, tail;
	T items[size];

	int inc(int val) {
		return (val + 1) % (size + 1);
	}
};

template<typename T, int size>
class Buffer {
public:
	Buffer() {
		for(int i = 0; i < size; i++) {
			free.add(&pool[i]);
		}
	}

	bool get_free(T** buffer) {

	}

	bool (T** buffer) {

	}


private:
	T pool[size];
	Queue<T*, size> free;
	Queue<T*, size> used;
};


TEST(Queue, add) {
	Queue<int, 5> q;
	ASSERT_TRUE(q.add(1));
};

TEST(Queue, underflow) {
	Queue<int, 5> q;
	int res;
	ASSERT_FALSE(q.get(&res));
};

TEST(Queue, add_get) {
	Queue<int, 5> q;
	int val;
	ASSERT_TRUE(q.add(100));
	ASSERT_TRUE(q.get(&val));
	ASSERT_EQ(100, val);
};

TEST(Queue, overflow) {
	Queue<int, 3> q;
	ASSERT_TRUE(q.add(100));
	ASSERT_TRUE(q.add(100));
	ASSERT_TRUE(q.add(100));
};

TEST(Queue, usecase) {
	Queue<int, 3> q;
	int val;
	ASSERT_TRUE(q.add(1));
	ASSERT_TRUE(q.add(2));
	ASSERT_TRUE(q.get(&val));
	ASSERT_EQ(1, val);
	ASSERT_TRUE(q.add(3));
	ASSERT_TRUE(q.get(&val));
	ASSERT_EQ(2, val);
	ASSERT_TRUE(q.get(&val));
	ASSERT_EQ(3, val);
};



TEST(Buffer, get) {
	Buffer<int, 10> buf;
	int *num;

	ASSERT_TRUE(buf.receive(&num));
	*num = 100;

	ASSERT_TRUE(buf.send(&num));



};