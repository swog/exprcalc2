#include <stdio.h>
#include "elex.h"

int main(int argc, char** argv) {
	if (argc < 2) {
		fprintf(stderr, "usage: %s <expression>\n", argv[0]);
		return 0;
	}

	double val;
	int r = ecalc(argv[1], val);	

	if (r != 0) {
		printf("err: %i\n", r);
		return 1;
	}

	printf("`%s`=%f\n", argv[1], val);
}
