#include "stdafx.h"
#include "tokens.h"
#include "syntaxtree.h"

static constexpr char GetPrescedence(char ch) {
	switch (ch) {
	case '^':
		return 4;
	case '*':
	case '/':
	case '%':
		return 3;
	case '+':
	case '-':
		return 2;
	}
	return 1;
}

static constexpr bool IsOperator(char ch) {
	return ch == '^' || ch == '*' || ch == '/' || ch == '+' || ch == '-' || ch == '(' || ch == ')' || ch == '%';
}

enum class TokenType : unsigned char {
	Empty,
	Literal = 1,
	Symbol,
};

static constexpr enum class TokenType TokenType(char ch) {
	return IsOperator(ch) ? TokenType::Symbol : TokenType::Literal;
}

class Token {
public:
	Token() {
	}

	Token(std::string_view Str) 
		: _Str(Str) {
	}

	inline char Front() const {
		return _Str.front();
	}

	inline enum class TokenType Type() const {
		return _Str.length() == 0 ? TokenType::Empty : TokenType(Front());
	}

	std::string_view _Str;
};

//https://stackoverflow.com/questions/423898/postfix-notation-to-expression-tree
class SyntaxTree : public Token {
public:
	SyntaxTree(const class Token& Token, SyntaxTree *Left, SyntaxTree *Right) {
		_Str = Token._Str;
		_Left = Left;
		_Right = Right;
	}

	SyntaxTree *_Left, *_Right;
};

class Lexer {
public:
	~Lexer() {
	}

	Lexer(const char *Input, size_t Size)
		: _Input(Input), _Index(0), _Size(Size) {
		if (!_Size)
			return;
		std::stack<Token> stack;
		std::vector<Token> postfix;
		Token token;
		do {
			token = Read();
			if (token.Type() == TokenType::Literal) {
				postfix.push_back(token);
			}
			else if (token.Type() == TokenType::Symbol) {
				if (token.Front() != ')') {
					while (token.Front() != '(' && stack.size() && GetPrescedence(stack.top().Front()) >= GetPrescedence(token.Front())) {
						postfix.push_back(stack.top());
						stack.pop();
					}
					stack.push(token);
				}
				else {
					while (stack.size() && stack.top().Front() != '(') {
						postfix.push_back(stack.top());
						stack.pop();
					}
					if (!stack.size() || stack.top().Front() != '(') {
						return; // ERROR expected ')'
					}
					stack.pop();
				}
			}
		} while (Advance());

		while (stack.size()) {
			postfix.push_back(stack.top());
			stack.pop();
		}

		std::stack<SyntaxTree> tree;

		for (const auto& Tok : postfix) {
			if (Tok.Type() == TokenType::Symbol) {
				Token Left = tree.top()._Str;
				tree.pop();
				Token Right = tree.top()._Str;
				tree.pop();
				tree.push(SyntaxTree(Tok, Left, Right));
			}
		}
	}

	const char *_Input;
	size_t _Index;
	size_t _Size;

private:
	bool Advance() {
		// March until we encounter a non-blank character
		for (; _Index < _Size && isblank(_Input[_Index]); _Index++);
		return _Index < _Size;
	}

	Token Read() {
		auto type = TokenType(_Input[_Index]);
		Token token;
		if (type == TokenType::Literal) {
			size_t i;
			for (i = _Index; i < _Size && type == TokenType(_Input[i]); i++);
			token = Token(std::string_view(_Input + _Index, i - _Index));
			_Index = i;
		}
		else if (type == TokenType::Symbol) {
			token = Token(std::string_view(_Input + _Index++, 1));
		}
		return token;
	}
};

int main() {
	const char str[] = "(A+B)/(C-D)";
	Lexer lex(str, sizeof(str));
}