#pragma once

const int MX = 85, MY = 85; // montazni bod kloubu na ose x, y
const int PX = 29, PY = 29; // delka paky na servu na ose x, y
const int CENTER_X_US = 1500;
const int CENTER_Y_US = 1500;
const double SHIFT_MM = 14;
const int MEASUREMENT_PERIOD_MS = 20;
// 30-16mm

const int SIZE_Y = 230;
const int SIZE_X = 170;

const int BOUNDARY_X = 10;
const int BOUNDARY_Y = 10;
const int PLANE_BOUNDARIES[] = {
		BOUNDARY_X, BOUNDARY_Y,
		SIZE_X - 2 * BOUNDARY_X, SIZE_Y - 2 * BOUNDARY_Y
};


//const int RminY = 10600, RmaxY = 44900;
//const int RminX = 8130, RmaxX = 50700;

const int RminX = 366, RmaxX = 3726;
const int RminY = 378, RmaxY = 3650;