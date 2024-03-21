#pragma once

// Maximum tokens to be parsed/frame
#define ELEX_MAXTOKENS 	256
// Maximum arguments given to functions
#define ELEX_MAXARGS	32

struct elex_token;

// Functions
// Passed arguments like: cos(pi), where the args would be {{"pi", 3.14159, etok_type_alpha}}
typedef int (*elex_fn)(elex_token& res, const std::vector<elex_token>& args);

// Literal
struct elex_token {
	std::string 	tok;
	// Numeric value
	double 		val;
	int 		prec;
	// Type of token
	etok_type 	type;
	// Found function pointer
	elex_fn 	fn;
}; 

#define TOK_SIZE 32

struct ecalc_state {
	std::unordered_map<std::string, double> *globals;
	std::unordered_map<std::string, elex_fn> *funcs;
	//std::vector<elex_token> postfix;
	std::stack<elex_token> vals;
	std::stack<elex_token> ops;
	etok_type type;
	etok_type ptype;
	int pdepth;
	double coeff;
	char tok[TOK_SIZE];
};

enum elex_err : int {
	elex_err_none,
	// Unknown operator
	elex_err_unknown = 1,
	// Unclosed parenthesis (frame)
	elex_err_unclosed = 2,
	// Too many tokens (safety measure)
	elex_err_big = 3,
	// Not enough tokens to pop from the stack to perform an operation
	elex_err_operand = 4,
	// Missing operator
	elex_err_operator = 5,
	// Misc tokenizer error
	elex_err_tokenizer = 6,
	// Used unknown global variable, or function.
	elex_err_global = 7,
	// Function comma syntax error
	elex_err_comma = 8,
	// Unreachable code	
	elex_err_unreachable = 9,
	// Unopened parenthesis
	elex_err_unopened = 10,
	// Custom function is a NULL pointer
	elex_err_nullfn = 11,
	// Missing argument
	elex_err_missingarg = 12,
	// Expected argument for function call
	elex_err_expectedarg = 13,
	// More than MAXARGS arguments
	elex_err_maxargs = 14,
};

int ecalc_ex(
	const std::string& str, size_t& pos, 
	elex_token& res, ecalc_state& s
); 

int ecalc(
	const std::string& str, double& res, 
	std::unordered_map<std::string, double>& globals,
	std::unordered_map<std::string, elex_fn>& funcs
);
