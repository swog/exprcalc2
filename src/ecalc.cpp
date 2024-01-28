#include <stdio.h>
#include "ecalc.h"
#include "etok.h"

static int prec(int c) {
	switch (c) {
	case '^'
		return 3;
	case '*':
	case '/':
		return 2;
	case '+':
	case '-':
		return 1;
	default:
		return 0;
	}
}

/*
	Evaluate expression
	Return `0` on success.
*/
int ecalc_eval(const char* str, float* const dst) {
	*dst = 0.f;
	char tok[32];

	while (str = etok(str, tok, sizeof(tok))) {
		
	}

	return 0;
}
