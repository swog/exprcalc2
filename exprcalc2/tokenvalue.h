#pragma once

//	Store all memory into this.
// Better for endless copies, as the current model does.
//	TokenValue stores type number `TokenType` and handles data accordingly.
// TokenValue uses the methods following `DestructTokenValue` to handle lvalue & rvalue
// assignments.
union TokenValueData {
	~TokenValueData() {}
	TokenValueData(double Num) : _Num(Num) {}
	TokenValueData(size_t Op) : _Op(Op) {}
	TokenValueData(const std::vector<class TokenValue>& Vec) : _Vec(Vec) {}
	TokenValueData(const std::vector<std::string_view>& Call) : _Call(Call) {}

	double							_Num;
	size_t							_Op;
	std::vector<class TokenValue>	_Vec;
	std::vector<std::string_view>	_Call;
};

//	Note: These methods do not explicitly change anything but the TokenValueData union.
// Furthermore, constructors and destructors are required to initialize _Type, _Neg explicitly
// before or after calling these methods.
void		DestructTokenValue(class TokenValue& Tok);
TokenValue& InitTokenValue(TokenValue& Tok);
TokenValue& CopyTokenValue(const TokenValue& From, TokenValue& To);
TokenValue& MoveTokenValue(TokenValue& From, TokenValue& To);

// The value of a token must be disconnected.
//	After beginning the SyntaxTree resolution, the connection between tokens
// and the input string becomes skewed.
class TokenValue {
public:
	// Parse static values
	// Variables will be parsed later in SyntaxTree, see SyntaxTree::Eval.
	TokenValue(class Lexer& Lexer, const class Token& Token, enum class TokenType Type);

	~TokenValue() {
		// Destroy if valid type
		DestructTokenValue(*this);
	}

	TokenValue()
		: _Type(TokenType::Empty), _Un(0.0), _Neg(false) {
		// Initialize to empty
	}

	TokenValue(const TokenValue& Other)
		: _Type(Other._Type), _Neg(Other._Neg), _Un(0.0) {
		// Initialize this type, then copy from `Other`
		InitTokenValue(*this);
		CopyTokenValue(Other, *this);
	}

	TokenValue(enum class TokenType Type)
		: _Type(Type), _Un(0.0), _Neg(false) {
		// Initialize for this type
		InitTokenValue(*this);
	}

	TokenValue(double Num)
		: _Type(TokenType::Number), _Un(Num), _Neg(false) {
	}

	TokenValue(const TokenValue& Other, bool Neg)
		: _Type(Other._Type), _Un(0.0), _Neg(Neg) {
		// Initialize to the other's type
		// Then copy with that type
		InitTokenValue(*this);
		CopyTokenValue(Other, *this);
	}

	TokenValue(const std::vector<std::string_view>& Call)
		: _Type(TokenType::FunctionCall), _Neg(false), _Un(Call) {
	}

	TokenValue(const std::vector<TokenValue>& Vec)
		: _Type(TokenType::Vector), _Un(Vec), _Neg(false) {
	}

	TokenValue& operator=(const TokenValue& Other) {
		// Free up this type
		DestructTokenValue(*this);
		// Copy type info to initialize & copy
		_Type = Other._Type;
		_Neg = Other._Neg;
		InitTokenValue(*this);
		CopyTokenValue(Other, *this);
		return *this;
	}

	TokenValue& operator=(TokenValue&& Other) noexcept {
		// Free up old memory via type
		DestructTokenValue(*this);
		// Copy type info to initialize
		_Type = Other._Type;
		_Neg = Other._Neg;
		InitTokenValue(*this);
		// Move over the data
		MoveTokenValue(Other, *this);
		// Nullify old type so it doesn't get released
		Other._Type = TokenType::Empty;
		//Other._Neg = false;
		return *this;
	}

	inline constexpr double GetNumber() const {
		return _Neg ? -_Un._Num : _Un._Num;
	}

	std::string ToString() const {
		switch (_Type) {
		case TokenType::Vector: {
			std::string s = "<";
			for (size_t i = 0; i < _Un._Vec.size(); i++) {
				s += _Un._Vec[i].ToString();
				if (i != _Un._Vec.size() - 1)
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

	typedef TokenValue(*TokenOp)(const TokenValue& Left, const TokenValue& Right);

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
		for (size_t i = 0; i < Left._Un._Vec.size(); i++) {
			ret._Un._Vec.push_back(Left._Un._Vec[i].PerformOp(Right, Op));
		}
		return ret;
	}

	static TokenValue PerformLitVec(const TokenValue& Left, const TokenValue& Right, TokenOp Op) {
		TokenValue ret(TokenType::Vector);
		for (size_t i = 0; i < Right._Un._Vec.size(); i++) {
			ret._Un._Vec.push_back(Left.PerformOp(Right._Un._Vec[i], Op));
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
	bool							_Neg;
	// Union containing all types
	TokenValueData					_Un;
};

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