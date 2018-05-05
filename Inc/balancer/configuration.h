#pragma once

struct Configuration {
	double const_p, const_d, const_i;
	bool disableServos = false;

	Configuration() noexcept {
/*
  		const_p = 0.00001;
		const_i = 0.00002;
		const_d = 0.000006;

		const_p = 0.00001;
		const_i = 0.000005;
		const_d = 0.000006;
 */
		const_p = 0.000005;
		const_i = 0.000005;
		const_d = 0.000006;
	}
};