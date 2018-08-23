#pragma once

struct Configuration {
	double const_p, const_d, const_i;
	bool disableServos = false;

	Configuration() noexcept {
		const_p = 0.000005;
		const_i = 0.000005;
		const_d = 0.000006;
	}
};