#pragma once

struct Configuration {
	double const_p, const_d, const_i;

	Configuration() {
		const_p = 0.00002;
		const_i = 0.00002;
		const_d = 0.00001;
	}
};
