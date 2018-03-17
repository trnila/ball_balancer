#pragma once

class ITouch {
public:
	virtual void read(int &X, int &Y) = 0;
};