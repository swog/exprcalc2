#include <cstdio>
#include <unordered_map>
#include <string>
#define _USE_MATH_DEFINES
#include <cmath>
#include "elex.h"

int main(int argc, char** argv) {
	if (argc < 2) {
		fprintf(stderr, "usage: %s <expression>\n", argv[0]);
		return 0;
	}

	double val;
	std::unordered_map<std::string, double> globals;
	globals["e"] = M_E;
	globals["pi"] = M_PI;

	int r = ecalc(argv[1], val, globals);	

	if (r != 0) {
		fprintf(stderr, "err: %i\n", r);
		return 1;
	}

	printf("`%s`=%f\n", argv[1], val);
}
