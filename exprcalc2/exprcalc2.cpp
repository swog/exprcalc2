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

int main() {
	const char str[] = "";
	Lexer lexer(LexerFlags::Verbose);
	std::vector<Token> postfix;
	lexer.SetString(str, sizeof(str));
	lexer.InfixToPostfix(postfix);
	auto tree = lexer.PostfixToSyntaxTree(postfix);
	if (tree) {
		std::cout << tree->Eval().ToString() << std::endl;
	}
} 