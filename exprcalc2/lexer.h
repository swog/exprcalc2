#pragma once

enum class LexerFlags : unsigned char {
	Normal,
	NoFunctionCalls		= (1 << 0),
	NoVectors			= (1 << 1),
	Verbose				= (1 << 2),
	PrintNestedTrees	= (1 << 3),
};

inline enum LexerFlags LexerFlags() {
	return LexerFlags::Normal;
}

inline constexpr enum LexerFlags operator|(enum LexerFlags Left, enum LexerFlags Right) {
	return (enum LexerFlags)((unsigned char)Left | (unsigned char)Right);
}

inline constexpr enum LexerFlags operator&(enum LexerFlags Left, enum LexerFlags Right) {
	return (enum LexerFlags)((unsigned char)Left & (unsigned char)Right);
}

inline constexpr enum LexerFlags& operator|=(enum LexerFlags& Left, enum LexerFlags Right) {
	Left = Left | Right;
	return Left;
}

inline constexpr bool IsLexerFlagSet(enum LexerFlags Flags, enum LexerFlags Flag) {
	return (Flags & Flag) == Flag;
}

struct LexerState {
	LexerState()
		: _Input(NULL), 
		_Index(0), _Size(0),
		_Flags(LexerFlags()),
		_Neg(false) {
	}

	const char* _Input;
	size_t		_Index;
	size_t		_Size;
	enum LexerFlags	_Flags;
	Token		_Prev;
	bool		_Neg;
};

class Lexer {
public:
	friend class SyntaxTree;

	Lexer(enum LexerFlags Flags = LexerFlags::Normal)
		: _Input(NULL), _Index(0), _Size(0), _Flags(Flags), _Neg(false) {
	}
	
	size_t InfixToPostfix(std::vector<class Token>& Postfix);

	// Must be a shared ptr because the stack holds a list of shared ptrs.
	//	The tree holds a list of shared ptrs because to make child pairs, we need to first access the top 
	// (creating a copy from reference), which cannot be done by unique ptrs
	std::shared_ptr<class SyntaxTree> 
	PostfixToSyntaxTree(
		const std::vector<Token>& Postfix, 
		bool Verbose = false
	);

	bool Advance() {
		// March until we encounter a non-blank character
		for (; _Index < _Size && _Input[_Index] && isblank(_Input[_Index]); _Index++);
		return _Index < _Size && _Input[_Index];
	}

	Token Read();

	bool IsNegative();

	size_t ReadFunctionName(std::string& Name);
	size_t ReadFunctionName(std::string_view& Name);
	size_t ReadExpressionList(std::vector<std::string_view>& Arguments, 
		char BeginToken, char EndToken, char DelimiterToken, bool Clear = true);

	void Reset() {
		_Index = 0;
		_Neg = false;
		_Prev.Type() = TokenType::Empty;
	}

	void SetString(
		const char* Str,
		size_t		Size
	) {
		_Input = Str;
		_Index = 0;
		_Neg = false;
		_Size = Size;
		_Prev.Type() = TokenType::Empty;
	}

	void SetString(
		const char*		Str, 
		size_t			Size,
		LexerState&		State
	) {
		Save(State);

		_Input = Str;
		_Index = 0;
		_Size = Size;
		_Neg = false;
		_Prev.Type() = TokenType::Empty;
	}

	void Restore(const LexerState& State) {
		_Input = State._Input;
		_Size = State._Size;
		_Index = State._Index;
		_Flags = State._Flags;
		_Prev = State._Prev;
		_Neg = State._Neg;
	}

	void Save(LexerState& State) const {
		State._Input = _Input;
		State._Size = _Size;
		State._Index = _Index;
		State._Prev = _Prev;
		State._Flags = _Flags;
		State._Neg = _Neg;
	}

	void SetIndex(size_t Index) {
		_Index = Index;
	}

	enum LexerFlags SetFlags(enum LexerFlags Flags) {
		auto flags = _Flags;
		_Flags = Flags;
		return flags;
	}

	enum LexerFlags AddFlags(enum LexerFlags Flags) {
		auto flags = _Flags;
		_Flags |= Flags;
		return flags;
	}

	bool IsFlagSet(enum LexerFlags Flags) const {
		return (unsigned char)(_Flags & Flags) != 0;
	}

	std::string_view ToString() const {
		return std::string_view(_Input, _Size);
	}

	size_t ParseOperator(const Token& Tok) const {
		size_t op = 0;
		char* pOp = (char*)&op;
		for (size_t i = 0; i < sizeof(op) && i < Tok._Str.size(); i++) {
			pOp[i] = Tok._Str[i];
		}
		return op;
	}

private:
	const char* _Input;
	size_t		_Index;
	size_t		_Size;
	enum LexerFlags	_Flags;
	Token		_Prev;
	bool		_Neg;

	// Count the call token length following a function name
	bool ExpressionLength(size_t& i, char BeginToken, char EndToken);
};