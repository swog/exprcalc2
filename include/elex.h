#pragma once

typedef double (*elex_evalfn)(double left, double right);

enum elex_err : int {
	elex_err_none,
	// Unknown operator
	elex_err_unknown,
	// Unclosed parenthesis (frame)
	elex_err_unclosed,
	// Too many tokens (safety measure)
	elex_err_big,
	// Not enough tokens to pop from the stack to perform an operation
	elex_err_operand,
	// Missing operator
	elex_err_operator,
	// Misc tokenizer error
	elex_err_tokenizer,
	// Used unknown global.
	elex_err_global,
};

typedef struct {
	int prec;
	elex_evalfn fn;
} elex_op;

int ecalc(const char* str, double& res, const std::unordered_map<std::string, double>& globals);
