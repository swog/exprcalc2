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

#define PREC_ADD 1
#define PREC_SUB 1
#define PREC_MUL 2
#define PREC_DIV 2

static double evaladd(double left, double right) {
	DbgPrintf("%f+%f\n", left, right);
	return left+right;
}

static elex_op addop = {PREC_ADD, evaladd};

static double evalsub(double left, double right) {
	DbgPrintf("%f-%f\n", left, right);
	return left-right;
}

static elex_op subop = {PREC_SUB, evalsub};

static double evalmul(double left, double right) {
	DbgPrintf("%f*%f\n", left, right);
	return left*right;
}

static elex_op mulop = {PREC_MUL, evalmul};

static double evaldiv(double left, double right) {
	DbgPrintf("%f/%f\n", left, right);
	return left/right;
}

static elex_op divop = {PREC_DIV, evaldiv};

static elex_op nulop = {0, NULL};

// Close a frame (parenthetical expression)
static int performop(
	const elex_op* 			op,
	std::stack<double>& 		vals,
	std::stack<const elex_op*>& 	ops,
	bool		 		push
) {
	// Perform operations
	while (!ops.empty() && ops.top()->prec >= op->prec) {
		// Until '(', a NULL function
		if (ops.top()->fn == NULL) {
			// Only pop frame `(` until `)`
			if (op == &nulop) {
				ops.pop();
			}

			break;
		}
		// There is not enough things to pop off the stack.
		else if (vals.size() < 2) {
			return elex_err_operand;
		}
		else {
			elex_evalfn fn = ops.top()->fn;

			double right = vals.top();
			vals.pop();

			double left = vals.top();
			vals.pop();

			vals.push(fn(left, right));
			ops.pop();
		}
	}

	if (push) {
		ops.push(op);
	}

	return elex_err_none;
}

// ecalc variable declaration warranted a state
typedef struct {
	// Position in the string input
	size_t pos;
	// Token precedence
	const elex_op* op;
	// Token
	char tok[32];
	// Current type
	etok_type type;
	// Previous type
	etok_type ptype;
	// Coefficient of the current alphanumeric token gathered from negatives
	double coeff;
	// Operator stack
	// NULL pointers are open parenthesis.
	std::stack<const elex_op*> ops;
	// Value stack
	std::stack<double> vals;
} ecalc_state;

int ecalc(const char* str, double& res) {
	size_t 	size;
	int	err;
	
	ecalc_state s;
	s.pos = 0;
	s.ptype = etok_type_null;
	s.coeff = 1.0;
	s.op = &nulop;

	//res = 0.0;
	size = strlen(str)+1;

	size_t i;

	for (i = 0; i < 256; i++) {
		err = etok(str, size, s.pos, s.tok, sizeof(s.tok), s.type);

		// Tokenizer error
		if (err != etok_err_none) {
			// EOS `errors` are ignored
			if (err != etok_err_eos) {
				return elex_err_tokenizer;
			}

			break;
		}
		
		// Push alphanumeric
		if (s.type == etok_type_alnum) {
			s.vals.push(s.coeff*atof(s.tok));
			s.coeff = 1.0;
		}
		else if (s.type == etok_type_punct) {
			switch (s.tok[0]) {	
			case '(':
				s.op = &nulop;
				s.ops.push(s.op);
				err = 0;
				break;
			case ')':
				s.op = &nulop;
				err = performop(s.op, s.vals, s.ops, false);
				break;
			case '-':
				// Negatives
				if (s.ptype != etok_type_alnum) {
					s.coeff *= -1.0;
					err = 0;
					break;
				}
				
				s.op = &subop;
				err = performop(s.op, s.vals, s.ops, true);
				break;	
			case '+':
				s.op = &addop;
				err = performop(s.op, s.vals, s.ops, true);
				break;
			case '*':
				s.op = &mulop;
				err = performop(s.op, s.vals, s.ops, true);
				break;
			case '/':
				s.op = &divop;
				err = performop(s.op, s.vals, s.ops, true);
				break;	
			default:
				return elex_err_operator;
			}

			if (err != 0) {
				return err;
			}
		}

		s.ptype = s.type;
	}

	if (i == 256) {
		return elex_err_big;
	}

	// Pop the rest of the arguments
	err = performop(&nulop, s.vals, s.ops, false);

	if (err) {
		return err;
	}

	// No values
	if (s.vals.empty()) {
		res = 0.0;
		return elex_err_none;
	}

	res = s.vals.top();

	return elex_err_none;
}
