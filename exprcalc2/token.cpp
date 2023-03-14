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
	: _Type(Type), _Neg(false), _Num(0.0) {
	if (_Type == TokenType::Number) {
		std::from_chars(Token._Str.data(), Token._Str.data() + Token._Str.size(), _Num);
	}
	else if (_Type == TokenType::FunctionCall) {
		const char* str; size_t size, index;
		Lexer.SetString(Token._Str.data(), Token._Str.length(), 
			&str, &size, &index);
		auto flags = Lexer.AddFlags(LexerFlags::NoFunctionCalls);

		std::string_view funcName;

		if (Lexer.ReadFunctionName(funcName)) {
			Lexer.Restore(str, size, index, flags);
			return;
		}
		
		_Call.push_back(funcName);

		Lexer.ReadExpressionList(_Call, '(', ')', ',', false);
		Lexer.Restore(str, size, index, flags);
	}
	else if (_Type == TokenType::Vector) {
		const char* str; size_t size, index;
		Lexer.SetString(Token._Str.data(), Token._Str.length(),
			&str, &size, &index);
		auto flags = Lexer.AddFlags(LexerFlags::NoVectors);

		std::vector<std::string_view> strArgs;

		Lexer.ReadExpressionList(strArgs, '<', '>', ',', false);
		EvalExprList(Lexer, strArgs, 0, _Vec);

		Lexer.Restore(str, size, index, flags);
	}
}

size_t EvalExprList(class Lexer& Lexer, const std::vector<std::string_view>& Expr, 
	size_t Start, std::vector<TokenValue>& Values) {
	size_t err = 0;
	std::vector<Token> postfix;
	
	// Save indices
	const char* str; size_t size, index; LexerFlags flags;
	Lexer.Save(&str, &size, &index, &flags);
	Lexer.SetFlags(LexerFlags::Normal);

	for (size_t i = Start; i < Expr.size(); i++) {
		Lexer.SetString(Expr[i].data(), Expr[i].size());

		if (Lexer.InfixToPostfix(postfix)) {
			std::cerr << "Warning: EvalExprList InfixToPostfix error\n";
			err = 1;
			break;
		}

		auto tree = Lexer.PostfixToSyntaxTree(postfix);

		if (tree) {
			Values.push_back(tree->Eval());
		}
		else {
			std::cerr << "Warning: Empty expression separated by commas\n";
			Values.push_back(0.0);
		}
	}

	Lexer.Restore(str, size, index, flags);
	return err;
}