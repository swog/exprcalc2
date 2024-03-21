#include <stack>
#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#include <cstring>
#include <memory>
#include "etok.h"
#include "elex.h"

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

#define PREC_PAREN 0
#define PREC_CONST 1
#define PREC_ADD 2
#define PREC_SUB 2
#define PREC_MUL 3
#define PREC_DIV 3

#define MAXARGS 32

static elex_token literal(ecalc_state& s, double val, elex_fn fn = NULL) {
	elex_token t;
	t.tok = s.tok;
	t.val = s.coeff*val;
	t.prec = PREC_CONST;
	t.type = s.type;
	t.fn = fn;
	return t;
}

static elex_token literal(double val) {
	elex_token t;
	t.val = val;
	t.prec = PREC_CONST;
	t.type = etok_type_num;
	t.fn = NULL;
	return t;
}

static elex_token operation(ecalc_state& s, int prec, elex_fn fn) {
	elex_token t;
	t.tok = s.tok;
	t.val = 0.0;
	t.prec = prec;
	t.type = s.type;
	t.fn = fn;
	return t;
}

static elex_token operation(const char* tok, int prec, etok_type type, elex_fn fn) {
	elex_token t;
	t.tok = tok;
	t.val = 0.0;
	t.prec = prec;
	t.type = type;
	t.fn = fn;
	return t;
}

#define ECALC_OP(NAME, OP) \
static int ecalc_##NAME(elex_token& res, const std::vector<elex_token>& args) { \
	if (args.size() < 2) return elex_err_operand; \
	res = literal(args[0].val OP args[1].val); \
	return 0; \
}

ECALC_OP(add, +);
ECALC_OP(sub, -);
ECALC_OP(mul, *);
ECALC_OP(div, /);


//
// 	Perform alphanumeric token
//
// Push each number to the `vals` stack.
// 
// If it's alphabetical then
//  
//  If it's a global then
//    Push global
//  
//  Else if it's a function then
//    Push function
//  
//  Else then
//    Error
//
//   Reset coefficient such that negations only
// affect one literal.
//
static int do_alnum(ecalc_state& s) {
	if (s.type == etok_type_num) {
		//s.postfix.push_back({s.tok, s.coeff*atof(s.tok), 0, s.type, NULL});
		s.vals.push(literal(s, atof(s.tok)));
	}
	else if (s.type == etok_type_alpha) {
		auto gmap = s.globals->find(s.tok);
		if (gmap != s.globals->cend()) {
			//s.postfix.push_back({s.tok, s.coeff*gmap->second, 0, s.type, NULL});
			s.vals.push(literal(s, gmap->second));
		}
		else {
			auto fmap = s.funcs->find(s.tok);
			if (fmap != s.funcs->cend()) {
				//s.postfix.push_back({s.tok, s.coeff, 0, s.type, fmap->second});
				s.vals.push(literal(s, 1.0, fmap->second));
			}
			else {
				return elex_err_global;
			}
		}	
	}
	
	s.coeff = 1.0;

	return 0;
}

//	Perform preceding operations, then push operator
// 
// For each operation in stack:
//   
//   If operation is an open parenthesis then
//     If the new operation is a close parenthesis then
//       Pop the open parenthesis
//     Else
//       Stop
//
//   Else
//
//     Pop left and right from the stack
//     Perform operation
//     Push result
//     Pop operation
//
//   Push new operation
// 
static int push_op(ecalc_state& s, int prec, elex_fn fn) {
	while (!s.ops.empty() && s.ops.top().prec >= prec) {
		if (s.ops.top().prec == PREC_PAREN) {
			// Remove `(` and don't add `)`
			if (prec == PREC_PAREN) {
				s.ops.pop();
				return 0;
			}
			else {
				// Push onto stack
				break;
			}
		}
		
		//s.postfix.push_back(s.ops.top());

		elex_token right = s.vals.top();
		s.vals.pop();		

		elex_token left = s.vals.top();
		s.vals.pop();

		elex_token res;

		int err = s.ops.top().fn(res, {left, right});

		if (err) {
			return err;
		}

		s.vals.push(res);
		s.ops.pop();
	}

	s.ops.push(operation(s, prec, fn));

	return 0;
}

static int do_punct(ecalc_state& s) {
	int err = 0;

	switch (s.tok[0]) {
	case '+':
		err = push_op(s, PREC_ADD, ecalc_add); 
		break;
	case '-':
		if (!(s.ptype & etok_type_alnum)) {
			s.coeff *= -1.0;
		}
		else {
			err = push_op(s, PREC_SUB, ecalc_sub);
		}
		break;
	case '*':
		err = push_op(s, PREC_MUL, ecalc_mul);
		break;
	case '/':
		err = push_op(s, PREC_DIV, ecalc_div);
		break;
	case '(':
		if (s.ptype & etok_type_alnum) {
			err = push_op(s, PREC_MUL, ecalc_mul);
		}

		s.pdepth++;
		s.ops.push(operation(s, PREC_PAREN, NULL));
		break;
	case ')':
		printf(") %d\n", s.pdepth);	
		if (s.pdepth <= 0) {
			return elex_err_unopened;
		}

		s.pdepth--;	
		err = push_op(s, PREC_PAREN, NULL);	
		break;
	// Function call break
	case ',':
		return elex_err_comma;
	default:
		return elex_err_unknown;
	}
	
	return err;
}

static int do_punct(ecalc_state& s, const char* punct) {
	strcpy(s.tok, punct);
	return do_punct(s);
}

static inline bool is_functioncall(ecalc_state& s) {
	return s.tok[0] == '(' && s.ptype == etok_type_alpha && s.vals.top().fn != NULL;
}

static ecalc_state copy_state(ecalc_state& s) {
	ecalc_state c;

	c.globals = s.globals;
	c.funcs = s.funcs;
	c.type = c.ptype = etok_type_null;
	c.coeff = 1.0;
	c.tok[0] = 0;
	c.pdepth = 0;

	return c;
}

static int do_functioncall(const std::string& str, size_t size, 
	size_t& pos, ecalc_state& s) {

	// Copy state
	ecalc_state new_s = copy_state(s);
	elex_token tmp = literal(0.0);
	std::vector<elex_token> args;

	int err = 0;
	size_t i;

	for (i = 0; i < MAXARGS && !err; i++) {
		err = ecalc_ex(str, pos, tmp, new_s);
		args.push_back(tmp);

		// End of function call `)`
		if (err == elex_err_unopened) {
			err = 0;
			break;
		}
		else if (err == elex_err_comma) {
			err = 0;
		}
	}

	if (i >= MAXARGS) {
		return elex_err_maxargs;	
	}

	// ecalc_ex won't get a EOS error; any error is bad
	if (err) {
		return err;
	}

	// Get the function token
	elex_token func = s.vals.top();
	s.vals.pop();

	// Call the function
	elex_token r;
	err = func.fn(r, args);

	// Push the return value
	s.vals.push(r);

	return err;
}

int ecalc_ex(
	const std::string& str, size_t& pos, 
	elex_token& res, ecalc_state& s
) {
	const size_t& size = str.size();
	int err;

	size_t i;

	do_punct(s, "(");

	for (i = 0; i < ELEX_MAXTOKENS; i++) {
		err = etok(str.c_str(), size, pos, s.tok, sizeof(s.tok), s.type);
	
		if (err == etok_err_eos) {
			err = 0;
			break;
		}

		// Alphanumerics
		if (s.type & etok_type_alnum) {
			err = do_alnum(s);	
		}
		else if (s.type == etok_type_punct) {
			// Function call
			if (is_functioncall(s)) {
				err = do_functioncall(str, size, pos, s);
			}
			else {
				err = do_punct(s);
			}
		}

		if (err) {
			break;
		}
		
		// Only used for negation
		s.ptype = s.type;
	}

	// > 256 tokens, fishy usage
	if (i >= ELEX_MAXTOKENS) {
		return elex_err_big;
	}

	printf("yes\n");
	do_punct(s, ")");

	// No operands
	if (s.vals.empty()) {
		return elex_err_operand;
	}

	res = s.vals.top();

	// Unclosed parenthesis
	if (s.pdepth > 0) {
		return elex_err_unclosed;
	}

	return err;
}

int ecalc(
	const std::string& str, double& res, 
	std::unordered_map<std::string, double>& globals,
	std::unordered_map<std::string, elex_fn>& funcs
) {
	size_t pos = 0;
	ecalc_state s;

	s.globals = &globals;
	s.funcs = &funcs;
	s.type = s.ptype = etok_type_null;
	s.coeff = 1.0;
	s.tok[0] = 0;
	s.pdepth = 0;

	elex_token res_token;

	int err = ecalc_ex(str, pos, res_token, s);

	if (err) {
		return err;
	}

	res = res_token.val;
	
	return 0;
}


