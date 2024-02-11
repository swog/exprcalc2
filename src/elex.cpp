#include <stack>
#include <vector>
#include <iostream>
#include <cstring>
#include "elex.h"
#include "etok.h"

// Src: etok.h
int etok(	const char* str, size_t size, size_t& pos, 
		char* tok, size_t tok_size, 
		etok_type& type
);

#ifdef DEBUG
#define DbgPrintf printf
#else
#define DbgPrintf
#endif

static double elex_evaladd(double left, double right) {
	DbgPrintf("%f+%f\n", left, right);
	return left+right;
}

static double elex_evalsub(double left, double right) {
	DbgPrintf("%f-%f\n", left, right);
	return left-right;
}

static double elex_evalmul(double left, double right) {
	DbgPrintf("%f*%f\n", left, right);
	return left*right;
}

static double elex_evaldiv(double left, double right) {
	DbgPrintf("%f/%f\n", left, right);
	return left/right;
}

static int elex_popframe(
	std::stack<double>& vals,
	std::stack<elex_evalfn>& ops
) {
	// Perform operations
	while (!ops.empty()) {
		// Until '(', a NULL function
		if (ops.top() == NULL) {
			ops.pop();
			break;
		}
		// There is not enough things to pop off the stack.
		else if (vals.size() < 2) {
			return 1;
		}
		else {
			elex_evalfn fn = ops.top();

			double left = vals.top();
			vals.pop();

			double right = vals.top();
			vals.pop();

			vals.push(fn(left, right));
			ops.pop();
		}
	}

	return 0;
}

// To fix the unary issue:
// This will be blazing fast...
// 1. We need to count op frames (0 := unary)
// 2. Keep previous token type (etok_type_punct := unary)
int ecalc(const char* str, double& res) {
	size_t 			size;
	size_t 			pos;
	
	int			err;
	char 			tok[32];
	etok_type 		type;

	std::stack<elex_evalfn> 	ops;
	std::stack<double>		val;

	res = 0.0;
	pos = 0;
	size = strlen(str)+1;

	size_t i;

	for (i = 0; i < 256; i++) {
		err = etok(str, size, pos, tok, sizeof(tok), type);

		if (err != etok_success) {
			break;
		}
		
		// Push alphanumeric
		if (type == etok_type_alnum) {
			val.push(atof(tok));
		}
		else if (type == etok_type_punct) {
			switch (tok[0]) {
			case '+':
				ops.push(elex_evaladd);
				break;
			case '-':
				val.push(-1.0);
				ops.push(elex_evalsub);
				break;
			case '*':
				ops.push(elex_evalmul);
				break;
			case '/':
				ops.push(elex_evaldiv);				
				break;
			case '(':
				ops.push(NULL);
				break;
			// I don't think we set fc=0 here
			// We need frame count to be zero only when '(' 
			case ')':
				elex_popframe(val, ops);
				break;
			default:
				return elex_err_unk;
			}
		}
	}

	if (i == 256) {
		return elex_err_tmt;
	}

	elex_popframe(val, ops);
	res = val.top();

	return (val.size() != 1 || !ops.empty()) ? elex_err_unclosed : elex_success;
}
