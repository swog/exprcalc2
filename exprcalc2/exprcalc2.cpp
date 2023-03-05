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

static constexpr size_t TokenType(char ch);

class Token {
public:
	enum : size_t {
		Literal = 1,
		Symbol,
	};

	Token() {
	}

	Token(std::string_view Str) 
		: _Str(Str) {
	}

	inline char Front() const {
		return _Str.front();
	}

	// 0 - Unknown, 1 - Operator, 2 - Literal
	inline size_t Type() const {
		return _Str.length() == 0 ? 0 : TokenType(Front());
	}

	std::string_view _Str;
};

static constexpr size_t TokenType(char ch) {
	return IsOperator(ch) ? Token::Symbol : Token::Literal;
}

class SyntaxTree {
public:
};

class Lexer {
public:
	Lexer(const char *Input, size_t Size)
		: _Input(Input), _Index(0), _Size(Size), _Tokens(NULL) {
		if (!_Size)
			return;
		std::stack<Token> stack;
		std::vector<Token> postfix;
		Token token;
		do {
			token = Read();
			if (token.Type() == Token::Symbol) {
				if (token.Front() == ')') {
					while (stack.size() && stack.top().Front() != '(') {
						postfix.push_back(stack.top());
						stack.pop();
					}
					if (!stack.size() || stack.top().Front() != '(') {
						return; // ERROR expected ')'
					}
					stack.pop();
				}
				else {
					while (token.Front() != '(' && stack.size() && GetPrescedence(stack.top().Front()) >= GetPrescedence(token.Front())) {
						postfix.push_back(stack.top());
						stack.pop();
					}
					stack.push(token);
				}
			}
			else if (token.Type() == Token::Literal) {
				postfix.push_back(token);
			}
		} while (Advance());

		while (stack.size()) {
			postfix.push_back(stack.top());
			stack.pop();
		}

		for (const auto& tok : postfix)
			std::cout << tok._Str << '\n';
	}

	const char *_Input;
	size_t _Index;
	size_t _Size;
	Token *_Tokens;

private:
	bool Advance() {
		// March until we encounter a non-blank character
		for (; _Index < _Size && isblank(_Input[_Index]); _Index++);
		return _Index < _Size;
	}

	Token Read() {
		size_t type = TokenType(_Input[_Index]);
		Token token;
		if (type == Token::Literal) {
			size_t i;
			for (i = _Index; i < _Size && type == TokenType(_Input[i]); i++);
			token = Token(std::string_view(_Input + _Index, i - _Index));
			_Index = i;
		}
		else if (type == Token::Symbol) {
			token = Token(std::string_view(_Input + _Index++, 1));
		}
		return token;
	}
};

int main() {
	const char str[] = "7*f(x,y,z)";
	Lexer lex(str, sizeof(str));

}