#pragma once

typedef TokenValue (*SyntaxCFunc)(const std::string& Name, 
	const std::vector<TokenValue>& Arguments);

// SyntaxTree
//
//	Syntax trees perform a recursive descent resolution.
// The Lexer creates tokens into an "abstract syntax tree." The lexer 
class SyntaxTree {
public:
	~SyntaxTree() {
		_Left.reset();
		_Right.reset();
	}

	SyntaxTree(const class Token& Token, 
		std::shared_ptr<SyntaxTree> Left, std::shared_ptr<SyntaxTree> Right) {
		_Token = Token;
		_Left = Left;
		_Right = Right;
	}

	void Print(size_t Depth = 0) {
		for (size_t i = 0; i < Depth; i++)
			std::cout << '\t';

		_Token.Print();

		Depth++;

		if (_Left)
			_Left->Print(Depth);

		if (_Right)
			_Right->Print(Depth);
	}

	TokenValue Eval() const;

	// Resolve variable from _Globals or parsed numeric
	TokenValue GetValue() const;

	Token _Token;
	std::shared_ptr<SyntaxTree> _Left, _Right;

	static std::unordered_map<std::string, TokenValue>	_Globals;
	static std::unordered_map<std::string, SyntaxCFunc>	_CFuncs;

private:
	TokenValue DoFunctionCall() const;
};