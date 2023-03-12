#pragma once

enum class TokenType : unsigned char {
	Empty,
	Number = 1,
	Word,
	Operator,
	FunctionCall,
	Vector,
};

inline constexpr enum class TokenType TokenType(char ch);

// The value of a token must be disconnected.
//	After beginning the SyntaxTree resolution, the connection between tokens
// and the input string becomes skewed.
class TokenValue {
public:
	// Parse static values
	// Variables will be parsed later in SyntaxTree
	TokenValue(class Lexer& Lexer, const class Token& Token, enum class TokenType Type);

	TokenValue()
		: _Type(TokenType::Empty), _Neg(false), _Num(0.0) {
	}

	TokenValue(double Num)
		: _Type(TokenType::Number), _Neg(false), _Num(Num) {
	}

	TokenValue(const std::vector<std::string_view>& Call)
		: _Type(TokenType::FunctionCall), _Neg(false), _Num(0.0), _Call(Call) {
	}

	TokenValue(const std::vector<TokenValue>& Vec)
		: _Type(TokenType::Vector), _Neg(false), _Num(0.0), _Vec(Vec) {
	}

	inline constexpr double GetNumber() const {
		return _Neg ? -_Num : _Num;
	}

	std::string ToString() const {
		switch (_Type) {
		case TokenType::Vector: {
			std::string s = "<";
			for (size_t i = 0; i < _Vec.size(); i++) {
				s += _Vec[i].ToString();
				if (i != _Vec.size() - 1)
					s += ", ";
			}
			s += ">";
			return s;
		}
		case TokenType::Number:
		case TokenType::Word:
			return std::to_string(GetNumber());
		}
		return "";
	}

	enum class TokenType			_Type;
	bool							_Neg;
	double							_Num;
	std::vector<TokenValue>			_Vec;
	std::vector<std::string_view>	_Call;
};

class Token {
public:
	static inline constexpr bool IsOperator(char ch) {
		return ch == '^' || ch == '*' || ch == '/' || ch == '+' || ch == '-' || 
			ch == '(' || ch == ')' || ch == '%' || ch == '<' || ch == '>' || ch == ',';
	}

	static char GetPrescedence(char ch);

	Token() {
	}

	Token(class Lexer& Lexer, std::string_view Str, enum class TokenType Type)
		: _Str(Str), _Value(Lexer, *this, Type) {
	}

	Token(double Num)
		: _Value(Num) {
	}

	inline char Front() const {
		return _Str.length() ? _Str.front() : 0;
	}

	inline enum class TokenType Type() const {
		return _Value._Type;
	}

	void Print() const;

	std::string_view	_Str;
	TokenValue			_Value;
};

inline constexpr enum class TokenType TokenType(char ch) {
	if (Token::IsOperator(ch))
		return TokenType::Operator;
	else if (isalpha(ch))
		return TokenType::Word;
	else if (isalnum(ch))
		return TokenType::Number;
	else
		return TokenType::Empty;
}

inline constexpr bool TokenTypeIsLiteral(enum class TokenType type) {
	return type == TokenType::Number || type == TokenType::Word;
}

// Some expression lists (Function calls) contain the name of the function as the first string
size_t EvalExprList(class Lexer& Lexer, const std::vector<std::string_view>& Expr, size_t Start, std::vector<TokenValue>& Values);