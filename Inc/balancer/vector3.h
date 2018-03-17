#pragma once
#include <cmath>

template<typename T>
struct Vector3 {
	T x;
	T y;
	T z;

	Vector3(T x, T y, T z): x(x), y(y), z(z) {}
	Vector3(): x(0), y(0), z(0) {}
};

template<typename T>
Vector3<T> normalize(const Vector3<T> &point) {
	double normalized = sqrt(point.x * point.x + point.y * point.y + point.z * point.z);

	if(fabs(normalized) < 0.0001f) {
		return point;
	}
	Vector3<T> p;
	p.x = point.x / normalized;
	p.y = point.y / normalized;
	p.z = point.z / normalized;
	return p;
}


template<typename T>
Vector3<T> operator-(const Vector3<T> &a, const Vector3<T> &b) {
	Vector3<T> result = a;
	result.x -= b.x;
	result.y -= b.y;
	result.z -= b.z;
	return result;
}

template<typename T>
Vector3<T> operator/(const Vector3<T> &a, T d) {
	Vector3<T> result = a;
	result.x /= d;
	result.y /= d;
	result.z /= d;
	return result;
}

template<typename T>
Vector3<T> operator*(T k, const Vector3<T>& v) {
	Vector3<T> copy = v;
	copy.x *= k;
	copy.y *= k;
	copy.z *= k;
	return copy;
}

template<typename T>
Vector3<T> operator+(const Vector3<T>& v1, const Vector3<T>& v2) {
	Vector3<T> copy = v1;
	copy.x += v2.x;
	copy.y += v2.y;
	copy.z += v2.z;
	return copy;
}

template<typename T>
Vector3<T> operator*(const Vector3<T>& v, T k) {
	return k * v;
}