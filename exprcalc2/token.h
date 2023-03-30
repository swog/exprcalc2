#pragma once

class Token {
public:
	static inline constexpr bool IsOperator(char ch) {
		return ch == '^' || ch == '*' || ch == '/' || ch == '+' || ch == '-' || 
			ch == '(' || ch == ')' || ch == '%' || ch == '<' || ch == '>' || ch == ',';
	}

	static inline constexpr bool IsSingleOperator(char ch) {
		return ch == '(' || ch == ')' || ch == '-';
	}

	static char GetPrescedence(char ch);
	
	bool Prescedes(const Token& Other) const {
		return GetPrescedence(Front()) >= GetPrescedence(Other.Front());
	}

	// Default constructors
	Token() {}

	// Initialize `TokenValue` via its constructor that parses that type.
	Token(class Lexer& Lexer, std::string_view Str, enum class TokenType Type)
		: _Str(Str), _Value(Lexer, *this, Type) {
	}

	Token(double Num)
		: _Value(Num) {
	}

	char Front() const {
		return _Str.length() ? _Str.front() : 0;
	}

	size_t GetOp() const {
		return _Value._Un._Op;
	}

	enum class TokenType Type() const {
		return _Value._Type;
	}

	enum class TokenType& Type() {
		return _Value._Type;
	}

	bool Negative() const {
		return _Value._Neg;
	}

	void Print() const;

	std::string_view	_Str;
	TokenValue			_Value;
};