#include "stdafx.h"
#include "syntaxtree.h"

std::unordered_map<std::string, TokenValue>	SyntaxTree::_Globals{
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

std::unordered_map<std::string, SyntaxCFunc> SyntaxTree::_CFuncs = {
	{"cos", SyntaxFuncs::cos},
	{"sin", SyntaxFuncs::sin},
	{"ln", SyntaxFuncs::ln},
	{"log", SyntaxFuncs::log},
};