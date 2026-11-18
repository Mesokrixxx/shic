#include "shic/math/util.h"

f64 math_pow10(i64 exponent) {
	f64 result = 1;
	f64 factor = exponent < 0 ? 0.1 : 10;
	u64 count = math_abs(exponent);

	while (count) {
		if (count & 1)
			result *= factor;

		factor *= factor;
		count >>= 1;
	}

	return result;
}
