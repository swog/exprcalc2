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
inline constexpr bool TokenTypeIsLiteral(const enum class TokenType type);

// The value of a token must be disconnected.
//	After beginning the SyntaxTree resolution, the connection between tokens
// and the input string becomes skewed.
class TokenValue {
public:
	// Parse static values
	// Variables will be parsed later in SyntaxTree
	TokenValue(class Lexer& Lexer, const class Token& Token, enum class TokenType Type);

	TokenValue()
		: _Type(TokenType::Empty), _Num(0.0), _Neg(false) {
	}

	TokenValue(const TokenValue& Other)
		: _Type(Other._Type), _Num(Other._Num), _Vec(Other._Vec), 
		_Call(Other._Call), _Neg(Other._Neg) {
	}

	TokenValue(enum class TokenType Type) 
		: _Type(Type), _Num(0.0), _Neg(false) {
	}

	TokenValue(double Num)
		: _Type(TokenType::Number), _Num(Num), _Neg(false) {
	}

	TokenValue(const TokenValue& Other, bool Neg)
		: _Type(Other._Type), _Num(Other._Num), _Vec(Other._Vec),
		_Call(Other._Call), _Neg(Neg) {
	}

	TokenValue(const std::vector<std::string_view>& Call)
		: _Type(TokenType::FunctionCall), _Num(0.0), _Call(Call), _Neg(false) {
	}

	TokenValue(const std::vector<TokenValue>& Vec)
		: _Type(TokenType::Vector), _Num(0.0), _Vec(Vec), _Neg(false) {
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

	typedef TokenValue (*TokenOp)(const TokenValue& Left, const TokenValue& Right);

	static TokenValue PerformLitMul(const TokenValue& Left, const TokenValue& Right) {
		return Left.GetNumber() * Right.GetNumber();
	}

	static TokenValue PerformLitDiv(const TokenValue& Left, const TokenValue& Right) {
		auto right = Right.GetNumber();
		if (round(right) == 0.0) {
			std::cerr << "Warning: Literal-literal divide by zero\n";
			return 0.0;
		}
		return Left.GetNumber() / right;
	}

	static TokenValue PerformLitAdd(const TokenValue& Left, const TokenValue& Right) {
		return Left.GetNumber() + Right.GetNumber();
	}

	static TokenValue PerformLitSubtract(const TokenValue& Left, const TokenValue& Right) {
		return Left.GetNumber() - Right.GetNumber();
	}

	static TokenValue PerformLitExp(const TokenValue& Left, const TokenValue& Right) {
		auto left = Left.GetNumber();
		if (round(left) == 0.0) {
			std::cerr << "Warning: Literal-literal zero base exponential\n";
			return 0.0;
		}
		return pow(left, Right.GetNumber());
	}

	// Performs a TokenOp per vector value
	static TokenValue PerformVecLit(const TokenValue& Left, const TokenValue& Right, TokenOp Op) {
		TokenValue ret(TokenType::Vector);
		for (size_t i = 0; i < Left._Vec.size(); i++) {
			ret._Vec.push_back(Left._Vec[i].PerformOp(Right, Op));
		}
		return ret;
	}

	static TokenValue PerformLitVec(const TokenValue& Left, const TokenValue& Right, TokenOp Op) {
		TokenValue ret(TokenType::Vector);
		for (size_t i = 0; i < Right._Vec.size(); i++) {
			ret._Vec.push_back(Left.PerformOp(Right._Vec[i], Op));
		}
		return ret;
	}

	TokenValue PerformOp(const TokenValue& Right, TokenOp LitLit) const {
		if (TokenTypeIsLiteral(_Type) && TokenTypeIsLiteral(Right._Type))
			return LitLit(*this, Right);
		else if (TokenTypeIsLiteral(_Type) && Right._Type == TokenType::Vector)
			return PerformLitVec(*this, Right, LitLit);
		else if (_Type == TokenType::Vector && TokenTypeIsLiteral(Right._Type))
			return PerformVecLit(*this, Right, LitLit);
		return 0.0;
	}
	
	TokenValue operator*(const TokenValue& Right) const {
		return PerformOp(Right, PerformLitMul);
	}

	TokenValue operator/(const TokenValue& Right) const {
		return PerformOp(Right, PerformLitDiv);
	}

	TokenValue operator+(const TokenValue& Right) const {
		return PerformOp(Right, PerformLitAdd);
	}

	TokenValue operator-(const TokenValue& Right) const {
		return PerformOp(Right, PerformLitSubtract);
	}

	TokenValue operator^(const TokenValue& Right) const {
		return PerformOp(Right, PerformLitExp);
	}

	enum class TokenType			_Type;
	double							_Num;
	bool							_Neg;
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
	
	bool Prescedes(const Token& Other) const {
		return GetPrescedence(Front()) >= GetPrescedence(Other.Front());
	}

	Token() {
	}

	Token(class Lexer& Lexer, std::string_view Str, enum class TokenType Type)
		: _Str(Str), _Value(Lexer, *this, Type) {
	}

	Token(double Num)
		: _Value(Num) {
	}

	char Front() const {
		return _Str.length() ? _Str.front() : 0;
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

inline constexpr bool TokenTypeIsLiteral(const enum class TokenType type) {
	return type == TokenType::Number || type == TokenType::Word;
}

// Expression lists are something like <1,2*2,3> or f(1,2*2,3)
// - Converts those into lists of TokenValues
// Some expression lists (Function calls) contain the name of the function as the first string
// IOW, The string_view expression list of a function call will have the function name
size_t EvalExprList(
	class Lexer&							Lexer,
	const std::vector<std::string_view>&	Expr,
	size_t									Start,
	bool									Neg,
	std::vector<TokenValue>&				Values
);

inline std::ostream& operator<<(std::ostream& Stream, const TokenValue& Value) {
	Stream << Value.ToString();
	return Stream;
}