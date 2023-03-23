#include "stdafx.h"
#include "token.h"
#include "syntaxtree.h"
#include "lexer.h"

std::unordered_map<std::string, TokenValue>	SyntaxTree::_Globals {
	{"pi", M_PI},
	{"e", M_E},
};

namespace SyntaxFuncs {
	static TokenValue cos(const std::string&, const std::vector<TokenValue>& Args) {
		if (Args.size() != 1 || !TokenTypeIsLiteral(Args[0]._Type)) {
			std::cerr << "Warning: Call to cosine with incorrect argument(s)\n";
			return 0.0;
		}
		return ::cos(Args[0].GetNumber());
	}
	static TokenValue sin(const std::string&, const std::vector<TokenValue>& Args) {
		if (Args.size() != 1 || !TokenTypeIsLiteral(Args[0]._Type)) {
			std::cerr << "Warning: Call to sine with incorrect argument(s)\n";
			return 0.0;
		}
		return ::sin(Args[0].GetNumber());
	}
	static TokenValue ln(const std::string&, const std::vector<TokenValue>& Args) {
		if (Args.size() != 1 || !TokenTypeIsLiteral(Args[0]._Type)) {
			std::cerr << "Warning: Call to ln with incorrect argument(s)\n";
			return 0.0;
		}
		return ::log(Args[0].GetNumber());
	}
	static TokenValue log(const std::string&, const std::vector<TokenValue>& Args) {
		if (Args.size() != 2 || !TokenTypeIsLiteral(Args[0]._Type) || !TokenTypeIsLiteral(Args[1]._Type)) {
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
static TokenValue Eval(const char (&Input)[_Size]) {
	static Lexer lexer;
	static std::vector<Token> postfix;
	lexer.SetString(Input, _Size);
	lexer.InfixToPostfix(postfix);
	auto tree = lexer.PostfixToSyntaxTree(postfix);
	if (tree) {
		return tree->Eval();
	}
	std::cerr << "Error: Empty tree\n";
	return 0.0;
}

std::ostream& operator<<(std::ostream& Stream, const TokenValue& Value) {
	Stream << Value.ToString();
	return Stream;
}

int main() {
	const char str[] = "-cos(pi)*2*-<1,2,,3>";
	std::cout << Eval(str) << std::endl;
} 