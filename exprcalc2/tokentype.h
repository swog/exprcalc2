#pragma once

enum class TokenType : unsigned char {
	Empty,
	Number = 1,
	Word,
	Operator,
	FunctionCall,
	Vector,
};

enum class TokenType
TokenType(
	char ch
);

bool
TokenTypeIsLiteral(
	enum class TokenType type
);