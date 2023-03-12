#include "stdafx.h"
#include "token.h"
#include "syntaxtree.h"
#include "lexer.h"

std::unordered_map<std::string, TokenValue>	SyntaxTree::_Globals {
	{"pi", M_PI},
	{"e", M_E},
	{"G", 0.00000000006673},
	{"g", 9.81},
};

namespace SyntaxFuncs {
	static TokenValue cos(const std::string& Name, const std::vector<TokenValue>& Arguments) {
		if (Arguments.size() != 1 || !TokenTypeIsLiteral(Arguments[0]._Type)) {
			std::cerr << "Warning: Call to cosine with incorrect argument(s)\n";
			return 0.0;
		}
		return ::cos(Arguments[0].GetNumber());
	}
};

std::unordered_map<std::string, SyntaxCFunc> SyntaxTree::_CFuncs {
	{"cos", SyntaxFuncs::cos},
};

int main() {
	const char str[] = "<5,1,2,5*cos(pi)>";
	Lexer lexer(LexerFlags::Verbose);
	std::vector<Token> postfix;
	lexer.SetString(str, sizeof(str));
	lexer.InfixToPostfix(postfix);
	auto tree = lexer.PostfixToSyntaxTree(postfix);
	if (tree) {
		tree->Print();
		std::cout << tree->Eval().ToString() << std::endl;
	}
} 