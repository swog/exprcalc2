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
		return _Str.front() == '\0' ? TokenType::Empty : TokenType(Front());
	}

	std::string_view _Str;
};

//https://stackoverflow.com/questions/423898/postfix-notation-to-expression-tree
class SyntaxTree : public Token {
public:
	~SyntaxTree() {
		_Left.reset();
		_Right.reset();
	}
	
	SyntaxTree(const class Token& Token, std::shared_ptr<SyntaxTree> Left, std::shared_ptr<SyntaxTree> Right) {
		_Str = Token._Str;
		_Left = Left;
		_Right = Right;
	}

	void Print(size_t Depth = 0) {
		for (size_t i = 0; i < Depth; i++)
			std::cout << '\t';
		std::cout << _Str << '\n';
		if (_Left)
			_Left->Print(++Depth);
		if (_Right)
			_Right->Print(Depth);
	}

	double Eval() {
		if (Type() == TokenType::Symbol && _Left && _Right) {
			switch (_Str.front()) {
			case '^':
				return pow(_Left->Eval(), _Right->Eval());
			case '*':
				return _Left->Eval() * _Right->Eval();
			case '/':
				return _Left->Eval() / _Right->Eval();
			case '+':
				return _Left->Eval() + _Right->Eval();
			case '-':
				return _Left->Eval() - _Right->Eval();
			}
		}
		else
			return atof(_Str.data());
	}

	std::shared_ptr<SyntaxTree> _Left, _Right;
};

class Lexer {
public:
	Lexer() : _Input(NULL), _Index(0), _Size(0) {
	}

	size_t InfixToPostfix(const char *Input, size_t Size, std::vector<Token>& Postfix) {
		_Input = Input;
		_Size = Size;
		_Index = 0;

		if (!_Size)
			return 1;

		std::stack<Token> stack;
		Token token;
		do {
			token = Read();
			if (token.Type() == TokenType::Literal) {
				Postfix.push_back(token);
			}
			else if (token.Type() == TokenType::Symbol) {
				if (token.Front() != ')') {
					while (token.Front() != '(' && stack.size() && GetPrescedence(stack.top().Front()) >= GetPrescedence(token.Front())) {
						Postfix.push_back(stack.top());
						stack.pop();
					}
					stack.push(token);
				}
				else {
					while (stack.size() && stack.top().Front() != '(') {
						Postfix.push_back(stack.top());
						stack.pop();
					}
					if (!stack.size() || stack.top().Front() != '(') {
						return 2;
					}
					stack.pop();
				}
			}
		} while (Advance());

		while (stack.size()) {
			Postfix.push_back(stack.top());
			stack.pop();
		}

		return 0;
	}

	// Must be a shared ptr because the stack holds a list of shared ptrs.
	//	The tree holds a list of shared ptrs because to make child pairs, we need to first access the top 
	// (creating a copy from reference), which cannot be done by unique ptrs
	std::shared_ptr<SyntaxTree> PostfixToSyntaxTree(const std::vector<Token>& Postfix) {
		std::stack<std::shared_ptr<SyntaxTree>> tree;

		for (const auto& Tok : Postfix) {
			if (Tok.Type() == TokenType::Symbol) {
				std::shared_ptr<SyntaxTree> Right = tree.top();
				tree.pop();
				std::shared_ptr<SyntaxTree> Left = tree.top();
				tree.pop();
				tree.push(std::make_shared<SyntaxTree>(Tok, Left, Right));
			}
			else if (Tok.Type() == TokenType::Literal)
				tree.push(std::make_shared<SyntaxTree>(Tok, nullptr, nullptr));
		}

		std::shared_ptr<SyntaxTree> Head = tree.top();
		tree.pop();

		return Head;
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
	const char str[] = "(3+4)*((5^6)/7)";
	Lexer lexer;
	std::vector<Token> postfix;
	lexer.InfixToPostfix(str, sizeof(str), postfix);
	auto tree = lexer.PostfixToSyntaxTree(postfix);
	tree->Print();
	std::cout << tree->Eval() << std::endl;
}