#pragma once

enum class LexerFlags : unsigned char {
	Normal,
	NoFunctionCalls,
	NoVectors,
	Verbose = 4,
	PrintNestedTrees,
};

inline constexpr enum class LexerFlags operator|(LexerFlags Left, LexerFlags Right) {
	return (LexerFlags)((unsigned char)Left | (unsigned char)Right);
}

inline constexpr enum class LexerFlags operator&(LexerFlags Left, LexerFlags Right) {
	return (LexerFlags)((unsigned char)Left & (unsigned char)Right);
}

inline constexpr enum class LexerFlags& operator|=(LexerFlags& Left, LexerFlags Right) {
	Left = Left | Right;
	return Left;
}

inline constexpr bool IsLexerFlagSet(LexerFlags Flags, LexerFlags Flag) {
	return (Flags & Flag) == Flag;
}

struct LexerState {
	const char* _Input;
	size_t		_Index;
	size_t		_Size;
	LexerFlags	_Flags;
	Token		_Prev;
	bool		_Neg;
};

class Lexer {
public:
	friend class SyntaxTree;

	Lexer(LexerFlags Flags = LexerFlags::Normal) 
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
	Token Peek();

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

	LexerFlags SetFlags(LexerFlags Flags) {
		auto flags = _Flags;
		_Flags = Flags;
		return flags;
	}

	LexerFlags AddFlags(LexerFlags Flags) {
		auto flags = _Flags;
		_Flags |= Flags;
		return flags;
	}

	bool IsFlagSet(LexerFlags Flags) const {
		return (unsigned char)(_Flags & Flags) != 0;
	}

private:
	const char* _Input;
	size_t		_Index;
	size_t		_Size;
	LexerFlags	_Flags;
	Token		_Prev;
	bool		_Neg;

	// Count the call token length following a function name
	bool ExpressionLength(size_t& i, char BeginToken, char EndToken);
};