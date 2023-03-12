#pragma once

enum class LexerFlags : unsigned char {
	Normal,
	NoFunctionCalls,
	NoVectors,
	Verbose = 4,
};

inline enum class LexerFlags operator|(LexerFlags Left, LexerFlags Right) {
	return (LexerFlags)((unsigned char)Left | (unsigned char)Right);
}

inline enum class LexerFlags operator&(LexerFlags Left, LexerFlags Right) {
	return (LexerFlags)((unsigned char)Left & (unsigned char)Right);
}

inline enum class LexerFlags& operator|=(LexerFlags& Left, LexerFlags Right) {
	Left = Left | Right;
	return Left;
}

class Lexer {
public:
	friend class SyntaxTree;

	Lexer(LexerFlags Flags = LexerFlags::Normal) 
		: _Input(NULL), _Index(0), _Size(0), _Flags(Flags) {
	}
	
	size_t InfixToPostfix(std::vector<class Token>& Postfix);

	// Must be a shared ptr because the stack holds a list of shared ptrs.
	//	The tree holds a list of shared ptrs because to make child pairs, we need to first access the top 
	// (creating a copy from reference), which cannot be done by unique ptrs
	static std::shared_ptr<class SyntaxTree> PostfixToSyntaxTree(const std::vector<Token>& Postfix);

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
	}

	void SetString(const char* Str, 
		size_t Size,
		const char** pStr = NULL,
		size_t* pSize = NULL,
		size_t* pIndex = NULL) {
		Save(pStr, pSize, pIndex);
		_Input = Str;
		_Index = 0;
		_Size = Size;
	}

	void Restore(const char* Str,
		size_t Size,
		size_t Index,
		LexerFlags Flags) {
		_Input = Str;
		_Size = Size;
		_Index = Index;
		_Flags = Flags;
	}

	void Save(const char** pStr, size_t* pSize, size_t* pIndex, LexerFlags* pFlags = NULL) {
		if (pStr)
			*pStr = _Input;
		if (pSize)
			*pSize = _Size;
		if (pIndex)
			*pIndex = _Index;
		if (pFlags)
			*pFlags = _Flags;
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

	// Count the call token length following a function name
	bool ExpressionLength(size_t& i, char BeginToken, char EndToken);
};