#pragma once
#include <cmath>

template<typename T>
struct Vector {
	T x;
	T y;

	Vector(T x, T y): x(x), y(y) {}
	Vector(): x(0), y(0) {}
};

template<typename T>
Vector<T> operator*(double k, const Vector<T>& v) {
	Vector<T> copy = v;
	copy.x *= k;
	copy.y *= k;
	return copy;
}

template<typename T>
Vector<T> operator*(const Vector<T>& v, double k) {
	return k * v;
}

template<typename T>
Vector<T> operator/(const Vector<T>& v, double k) {
	Vector<T> copy = v;
	copy.x /= k;
	copy.y /= k;
	return copy;
}

template<typename T>
Vector<T> operator+(const Vector<T>& v1, const Vector<T>& v2) {
	Vector<T> copy = v1;
	copy.x += v2.x;
	copy.y += v2.y;
	return copy;
}

template<typename T>
Vector<T> operator-(const Vector<T>& v1, const Vector<T>& v2) {
	Vector<T> copy = v1;
	copy.x -= v2.x;
	copy.y -= v2.y;
	return copy;
}


typedef Vector<double> Vectorf;
typedef Vector<int> Vectori;

template<typename T>
bool vectorComparator(const Vector<T> &a, const Vector<T> &b) {
	if(a.x < b.x) {
		return true;
	}

	if(a.x == b.x) {
		return a.y < b.y;
	}

	return false;
}

template<typename T>
bool operator<(Vector<T> a, Vector<T> b) {
	return vectorComparator(a, b);
}

template<typename T>
Vector<T> normalize(const Vector<T> &point) {
	double normalized = sqrt(point.x * point.x + point.y * point.y);

	if(fabs(normalized) < 0.0001f) {
		return point;
	}
	Vector<T> p;
	p.x = point.x / normalized;
	p.y = point.y / normalized;
	return p;
}