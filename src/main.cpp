#include <cstdio>
#include <unordered_map>
#include <vector>
#include <string>
#include <stack>
#define _USE_MATH_DEFINES
#include <cmath>
#include "etok.h"
#include "elex.h"

static int eval_sin(elex_lit& res, std::vector<elex_lit>& args) {
	if (args.empty()) {
		return elex_err_missingarg;
	}

	res.val = sin(args[0].val);

	return 0;
}

int main(int argc, char** argv) {
	if (argc < 2) {
		fprintf(stderr, "usage: %s <expression>\n", argv[0]);
		return 0;
	}

	double val;
	std::unordered_map<std::string, double> globals;
	std::unordered_map<std::string, elex_fn> funcs;
	globals["e"] = M_E;
	globals["pi"] = M_PI;
	funcs["sin"] = eval_sin;

	int r = ecalc(argv[1], val, globals, funcs);	

	if (r != 0) {
		fprintf(stderr, "err: %i\n", r);
		return 1;
	}

	printf("`%s`=%f\n", argv[1], val);
}
