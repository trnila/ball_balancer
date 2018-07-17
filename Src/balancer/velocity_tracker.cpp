#include "balancer/utils.h"
#include "balancer/config.h"
#include "balancer/velocity_tracker.h"

bool VelocityTracker::update(int RX, int RY) {
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
