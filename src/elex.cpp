#include <stack>
#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#include <cstring>
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

static elex_op nulop = {0, NULL};

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


// Close a frame (parenthetical expression)
static int performop(
	const elex_op* 			op,
	std::stack<elex_lit>& 		vals,
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

			double right = vals.top().val;
			vals.pop();

			double left = vals.top().val;
			vals.pop();

			vals.push({"", fn(left, right), etok_type_num});
			ops.pop();
		}
	}

	if (push) {
		ops.push(op);
	}

	return elex_err_none;
}

int ecalc_ex(
	const std::string& str, size_t& pos, double& res, 
	const std::unordered_map<std::string, double>& globals,
	const std::unordered_map<std::string, elex_fn>& funcs,
	unsigned char fdepth
) {
	const size_t& 	size = str.size();
	int	err;
	
	ecalc_state s;
	s.ptype = etok_type_null;
	s.coeff = 1.0;
	s.op = &nulop;
	s.pdepth = 0;

	size_t i;

	for (i = 0; i < ELEX_MAXTOKENS; i++) {
		err = etok(str.c_str(), size, pos, s.tok, sizeof(s.tok), s.type);

		// Tokenizer error
		if (err != etok_err_none) {
			// EOS `errors` are ignored
			if (err != etok_err_eos) {
				return elex_err_tokenizer;
			}

			break;
		}
		
		// Push alphanumeric
		if (s.type == etok_type_alpha) {
			const auto gmap = globals.find(s.tok);

			// Functions?
			if (gmap == globals.cend()) {
				const auto fmap = funcs.find(s.tok);				

				if (fmap == funcs.cend()) {
					return elex_err_global;
				}
	
				// Handle it at `(`	
				// Store the function so we don't have to search again.
				s.vals.push({s.tok, s.coeff, etok_type_alpha, fmap->second});
			}	
			else {
				s.vals.push({s.tok, s.coeff*gmap->second, etok_type_num});
			}

			s.coeff = 1.0;
		}
		else if (s.type == etok_type_num) {
			// This works for negatives, reset coefficient
			s.vals.push({s.tok, s.coeff*atof(s.tok), etok_type_num});
			// Punctuation parsing will keep track of the actual negative value.
			// However, only use it once.
			s.coeff = 1.0;	
		}
		// Punctuation; operators
		else if (s.type == etok_type_punct) {
			// The tokenizer spits out 1 character long punctuation tokens
			switch (s.tok[0]) {	
			// Don't pop from the ops stack, only push `(`
			// This marks the end of the frame in the stack.
			case '(':
				// Alphanumeric previous value means function call.
				// Recursive call to ecalc
				if (s.ptype == etok_type_alpha) {
					// This shouldn't ever happen
					// Every alphanumeric is added to the stack.
					//   The only way this could happen is the global was not found, 
					// and it somehow didn't error
					if (s.vals.empty()) {
						return elex_err_unreachable;
					}

					// Custom function is a NULL pointer
					if (!s.vals.top().fn) {
						return elex_err_nullfn;
					}

					std::vector<elex_lit> args;
					elex_lit arg;
	
					// Maximum arguments
					size_t j;

					for (j = 0; j < ELEX_MAXARGS; j++) {
						err = ecalc_ex(str, pos, arg.val, globals, funcs, fdepth+1);

						if (err != elex_err_unopened) {
							return err;
						}
						
						args.push_back(arg);

						if (err == elex_err_unopened) {
							break;
						}
					}

					if (j > ELEX_MAXARGS) {
						return elex_err_maxargs;
					}

					double coeff = s.vals.top().val;
					err = s.vals.top().fn(s.vals.top(), args);	
					s.vals.top().val *= coeff;

					if (err) {
						return err;
					}
				}
				// Non-alphanumeric
				else {
					s.pdepth++;
					s.op = &nulop;
					s.ops.push(s.op);
					err = 0;
				}

				break;
			case ',':
				//   Depth either went negative somehow, or we're not in fd>0 
				// therefore we're not in a function..
				if (fdepth <= 0) {
					// The only general syntax error!
					//   I decided I don't want to call it err_syntax because
					// a syntax error could also be an unclosed function call.
					return elex_err_comma;
				}
	
				// Exit out	
				goto function_out;
			case ')':
				s.op = &nulop;
				err = performop(s.op, s.vals, s.ops, false);

				if (err) {
					return err;
				}

				// Exit out
				// The depth is 0 so not in an expression.
				// We're in a function call	
				if (s.pdepth <= 0) {
					if (s.vals.empty()) {
						return elex_err_expectedarg;
					}					

					res = s.vals.top().val;
					
					return elex_err_unopened;	
				}
		
				s.pdepth--;
				
				break;
			case '-':
				// Negatives
				// Multiply coefficient by -1 to keep track of multiple
				// -negatives.
				if (s.ptype != etok_type_alnum) {
					s.coeff *= -1.0;
					err = 0;
					break;
				}
				
				s.op = &subop;
				err = performop(s.op, s.vals, s.ops, true);
				break;	
			//   These are simple performops that perform everything
			// with >= precedence with the operation. Then push the
			// current operation.
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
			// Unknown operation
			default:
				return elex_err_operator;
			}

			if (err != 0) {
				return err;
			}
		}

		// Only used for negation
		s.ptype = s.type;
	}

	// > 256 tokens, fishy usage
	if (i > ELEX_MAXTOKENS) {
		return elex_err_big;
	}

	// pop the rest of the arguments
	err = performop(&nulop, s.vals, s.ops, false);

	if (err) {
		return err;
	}

	// No values; not an error just 0
	if (s.vals.empty()) {
		res = 0.0;
		return elex_err_none;
	}

	res = s.vals.top().val;

	return elex_err_none;
}

int ecalc(
	const std::string& str, double& res, 
	const std::unordered_map<std::string, double>& globals,
	const std::unordered_map<std::string, elex_fn>& funcs,
	unsigned int fdepth	// Function depth	
) {
	size_t pos = 0;
	
	return ecalc_ex(str, pos, res, globals, funcs, fdepth);
}


