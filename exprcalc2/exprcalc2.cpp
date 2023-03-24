//
//	ToDo: Make a more memory efficient way of transferring TokenValues.
// ATM, they copy two mostly useless vectors for simple literal-literal
// operations.
//	Sol: std::move it's contents, but this may break some things.
// 
//	Lexer tokenizes and reorders infix equations into `postfix`, where
// operators come after operands. This allows for operator prescedence, 
// and conversion to a syntax tree. Variables, functions, vectors, etc 
// are all resolved in correct order from the leaves of the tree to the root.
// This yields the result to the expression in the form of a TokenValue.
//	A TokenValue is a placeholder for numerous different possible types,
// including vectors. It has type-defined operator overloads, making 
// it easier to define new types.
// 
// Negatives were handled first in the Lexer class, with the member 
// variable `_Neg`, which determines if a token is negative or not.
// This state is then transferred to a TokenValue's `_Neg` member. This
// variable is used during SyntaxTree's recursive resolution - See SyntaxTree::Eval.
//	Negatives are mainly done in `EvalExprList`, which takes a `Neg` parameter,
// This parameter can be false, as is the case when a function call is done, and
// the arguments to said call shall not be negated. However, during a vector
// evaluation of the expression list, the negative state of the lexer is passed
// to the vector evaluation.
//

#include "stdafx.h"
#include "token.h"
#include "syntaxtree.h"
#include "lexer.h"

std::unordered_map<std::string, TokenValue>	SyntaxTree::_Globals {
	{"pi", M_PI},
	{"e", M_E},
};

namespace SyntaxFuncs {
	static DefineSyntaxCFunc(cos) {
		if (Args.size() != 1 || !TokenTypeIsLiteral(Args[0]._Type)) {
			std::cerr << "Warning: Call to cosine with incorrect argument(s)\n";
			return 0.0;
		}
		return ::cos(Args[0].GetNumber());
	}

	static DefineSyntaxCFunc(sin) {
		if (Args.size() != 1 || !TokenTypeIsLiteral(Args[0]._Type)) {
			std::cerr << "Warning: Call to sine with incorrect argument(s)\n";
			return 0.0;
		}
		return ::sin(Args[0].GetNumber());
	}

	static DefineSyntaxCFunc(ln) {
		if (Args.size() != 1 || !TokenTypeIsLiteral(Args[0]._Type)) {
			std::cerr << "Warning: Call to ln with incorrect argument(s)\n";
			return 0.0;
		}
		return ::log(Args[0].GetNumber());
	}
	
	static DefineSyntaxCFunc(log) {
		if (Args.size() != 2 
			|| !TokenTypeIsLiteral(Args[0]._Type) 
			|| !TokenTypeIsLiteral(Args[1]._Type)) {
			std::cerr << "Warning: Call to log with incorrect argument(s)\n";
			return 0.0;
		}

		return ::log(Args[1].GetNumber()) / ::log(Args[0].GetNumber());
	}
};

std::unordered_map<std::string, SyntaxCFunc> SyntaxTree::_CFuncs {
	{"cos", SyntaxFuncs::cos},
	{"sin", SyntaxFuncs::sin},
	{"ln", SyntaxFuncs::ln},
	{"log", SyntaxFuncs::log},
};

template<size_t _Size>
static TokenValue 
Eval(
	const char (&Input)[_Size], 
	enum LexerFlags Flags = LexerFlags::Normal
) {
	static Lexer lexer;
	static std::vector<Token> postfix;
	lexer.SetFlags(Flags);
	lexer.SetString(Input, _Size);
	lexer.InfixToPostfix(postfix);
	auto tree = lexer.PostfixToSyntaxTree(postfix);
	if (tree) {
		return tree->Eval();
	}
	std::cerr << "Error: Empty tree\n";
	return 0.0;
}

int main() {
	const char str[] = "-cos(pi)*2*-<1,2*-2,,3>";
	std::cout << Eval(str) << std::endl;
} 