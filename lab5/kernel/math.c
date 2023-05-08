#include "math.h"

int pow(int base, int pow) {
	int ret = 1, tmp = base;
	for (; pow; pow >>= 1, tmp *= tmp) {
		if (pow & 1)
			ret *= tmp;
	}
	return ret;
}
