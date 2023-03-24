#include "stdafx.h"
#include "lexer.h"
#include "syntaxtree.h"

char Token::GetPrescedence(char ch) {
	switch (ch) {
	case '^':
		return 4;
	case '*':
	case '/':
	case '%':
		return 3;
	case '+':
	case '-':
		return 2;
	}
	return 1;
}

void Token::Print() const {
	switch (_Value._Type) {
	case TokenType::Word:
		std::cout << _Str << ": ";
	case TokenType::Number:
		std::cout << _Value.GetNumber() << '\n';
		break;
	case TokenType::Operator:
		std::cout << Front() << '\n';
		break;
	case TokenType::FunctionCall:
	case TokenType::Vector:
		std::cout << _Str << '\n';
		break;
	}
}

TokenValue::TokenValue(class Lexer& Lexer, const Token& Token, enum class TokenType Type)
	: _Type(Type), _Num(0.0), _Neg(Token._Value._Neg) {
	if (_Type == TokenType::Number) {
		std::from_chars(Token._Str.data(), Token._Str.data() + Token._Str.size(), _Num);
	}
	else if (_Type == TokenType::FunctionCall) {
		LexerState state;
		Lexer.SetString(Token._Str.data(), Token._Str.length(), state);
		Lexer.AddFlags(LexerFlags::NoFunctionCalls);

		std::string_view funcName;

		if (Lexer.ReadFunctionName(funcName)) {
			Lexer.Restore(state);
			return;
		}
		
		_Call.push_back(funcName);

		Lexer.ReadExpressionList(_Call, '(', ')', ',', false);
		Lexer.Restore(state);
	}
	else if (_Type == TokenType::Vector) {
		LexerState state;
		Lexer.SetString(Token._Str.data(), Token._Str.length(), state);
		Lexer.AddFlags(LexerFlags::NoVectors);

		std::vector<std::string_view> strArgs;

		Lexer.ReadExpressionList(strArgs, '<', '>', ',', false);
		EvalExprList(Lexer, strArgs, 0, state._Neg, _Vec);
		Lexer.Restore(state);
	}
}

size_t EvalExprList(
	class Lexer&							Lexer,
	const std::vector<std::string_view>&	Expr,
	size_t									Start,
	bool									Neg,
	std::vector<TokenValue>&				Values
) {
	size_t err = 0;
	std::vector<Token> postfix;
	
	// Save indices
	LexerState state;
	Lexer.Save(state);
	bool verbose = IsLexerFlagSet(Lexer.SetFlags(LexerFlags::Normal), LexerFlags::Verbose);

	for (size_t i = Start; i < Expr.size(); i++) {
		Lexer.SetString(Expr[i].data(), Expr[i].size());

		if (Lexer.InfixToPostfix(postfix)) {
			if (verbose) {
				std::cerr << "Warning: EvalExprList InfixToPostfix error\n";
			}
			err = 1;
		}

		auto tree = Lexer.PostfixToSyntaxTree(postfix, verbose);

		if (tree) {
			auto res = tree->Eval();
			res._Neg = Neg;

			Values.push_back(res);

			if (IsLexerFlagSet(state._Flags, LexerFlags::PrintNestedTrees)) {
				std::cout << "Nested tree: " << Lexer.ToString() << '\n';
				tree->Print();
				std::cout << "==" << res;
				if (Neg) {
					std::cout << " (Negated)\n";
				}
			}
		}
		else {
			if (verbose) {
				std::cerr << "Warning: Empty expression separated by commas\n";
			}
			Values.push_back(0.0);
		}
	}

	Lexer.Restore(state);
	return err;
}