#pragma once

// Maximum tokens to be parsed/frame
#define ELEX_MAXTOKENS 	256
// Maximum arguments given to functions
#define ELEX_MAXARGS	32

// Precedence values
#define PREC_ADD 1
#define PREC_SUB 1
#define PREC_MUL 2
#define PREC_DIV 2
#define PREC_COM 0

typedef struct _elex_lit elex_lit;

// Functions
// Passed arguments like: cos(pi), where the args would be {{"pi", 3.14159, etok_type_alpha}}
typedef int (*elex_fn)(elex_lit& res, std::vector<elex_lit>& args);

// Literal
typedef struct _elex_lit {
	std::string 	tok;
	// Numeric value
	double 		val;
	// Type of token
	etok_type 	type;
	// Found function pointer
	elex_fn 	fn;
} elex_lit;

// Operators
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
	// Used unknown global variable, or function.
	elex_err_global,
	// Function comma syntax error
	elex_err_comma,
	// Unreachable code	
	elex_err_unreachable,
	// Unopened parenthesis
	elex_err_unopened,
	// Custom function is a NULL pointer
	elex_err_nullfn,
	// Missing argument
	elex_err_missingarg,
	// Expected argument for function call
	elex_err_expectedarg,
	// More than MAXARGS arguments
	elex_err_maxargs,
};

typedef struct {
	unsigned int 	prec;
	elex_evalfn 	fn;
} elex_op;

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
	//   Coefficient of the current alphanumeric token 
	// gathered from negatives
	double coeff;
	// Operator stack
	// NULL pointers are open parenthesis.
	std::stack<const elex_op*> ops;
	// Value stack
	std::stack<elex_lit> vals;
	// Parentheses depth
	unsigned int pdepth;
} ecalc_state;

int ecalc(
	const std::string& str, double& res, 
	const std::unordered_map<std::string, double>& globals,
	const std::unordered_map<std::string, elex_fn>& funcs,
	unsigned int fdepth = 0	// Function depth
);
