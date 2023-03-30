#include "stdafx.h"

enum class TokenType TokenType(char ch) {
	if (Token::IsOperator(ch))
		return TokenType::Operator;
	else if (isalpha(ch))
		return TokenType::Word;
	else if (isalnum(ch))
		return TokenType::Number;
	else
		return TokenType::Empty;
}

bool TokenTypeIsLiteral(enum class TokenType type) {
	return type == TokenType::Number || type == TokenType::Word;
}