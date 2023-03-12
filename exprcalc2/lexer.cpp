#include "stdafx.h"
#include "lexer.h"
#include "syntaxtree.h"

size_t Lexer::InfixToPostfix(std::vector<Token>& Postfix) {
	if (!_Size) {
		std::cerr << "Warning: InfixToPostfix on empty string\n";
		return 1;
	}

	Postfix.clear();

	bool neg = false;

	std::stack<Token> stack;
	Token token;
	while (Advance()) {
		token = Read();

		if (token.Type() == TokenType::Empty) {
			continue;
		}

		if (token.Type() != TokenType::Operator) {
			token._Value._Neg = neg;
			neg = false;
			Postfix.push_back(token);
		}
		else {
			// Negations are only when followed by an operator
			if (token.Front() == '-' && IsNegative()) {
				neg = !neg;
			}
			// Operators
			else if (token.Front() != ')') {
				// Push all operators into postfix by prescedence.
				// This makes higher prescedence operations be performed first
				while (token.Front() != '(' && stack.size() 
					&& Token::GetPrescedence(stack.top().Front()) >= Token::GetPrescedence(token.Front())) {
					Postfix.push_back(stack.top());
					stack.pop();
				}
				// Push the current operation
				stack.push(token);
			}
			// Close parentheses
			else {
				// Push all operators into postfix in the correct order
				while (stack.size() && stack.top().Front() != '(') {
					Postfix.push_back(stack.top());
					stack.pop();
				}
				// The parenthesis was not closed
				if (!stack.size() || stack.top().Front() != '(') {
					std::cerr << "Warning: Expected `)`\n";
					return 2;
				}
				// Pop the left parenthesis
				stack.pop();
			}
		}

		_Prev = token;
	}

	while (stack.size()) {
		Postfix.push_back(stack.top());
		stack.pop();
	}

	// Remove all remaining parentheses not resolved
	size_t unclosed = 0;

	for (size_t i = 0; i < Postfix.size(); i++) {
		if (Postfix[i].Front() == '(') {
			unclosed++;
			Postfix[i]._Value = 0.0;
		}
	}

	if (unclosed && IsFlagSet(LexerFlags::Verbose)) {
		std::cerr << "Warning: Unclosed parentheses (" << unclosed << ")\n";
	}

	return 0;
}

// Must be a shared ptr because the stack holds a list of shared ptrs.
//	The tree holds a list of shared ptrs because to make child pairs, we need to first access the top 
// (creating a copy from reference), which cannot be done by unique ptrs
std::shared_ptr<SyntaxTree> Lexer::PostfixToSyntaxTree(const std::vector<Token>& Postfix) {
	// This would fault if `Postfix` had no tokens
	if (!Postfix.size()) {
		std::cerr << "Warning: PostfixToSyntaxTree empty token vector\n";
		return nullptr;
	}

	std::stack<std::shared_ptr<SyntaxTree>> tree;

	for (const auto& Tok : Postfix) {
		if (Tok.Type() != TokenType::Empty && Tok.Type() != TokenType::Operator) {
			tree.push(std::make_shared<SyntaxTree>(Tok, nullptr, nullptr));
		}
		else if (Tok.Type() == TokenType::Operator && tree.size() >= 2) {
			std::shared_ptr<SyntaxTree> Right = tree.top();
			tree.pop();

			std::shared_ptr<SyntaxTree> Left = tree.top();
			tree.pop();

			tree.push(std::make_shared<SyntaxTree>(Tok, Left, Right));
		}
	}

	std::shared_ptr<SyntaxTree> Head = tree.top();
	tree.pop();

	return Head;
}

Token Lexer::Read() {
	auto type = TokenType(_Input[_Index]);
	Token token;
	// Word or number
	// `<` hacky fix for a vector that doesn't start with `vec<`
	if (TokenTypeIsLiteral(type) || _Input[_Index] == '<') {
		size_t i;

		// Match until type change
		for (i = _Index + 1; i < _Size && _Input[i]; i++) {
			if (type != TokenType(_Input[i])) {
				break;
			}
		}

		// A literal alphabetical followed by a `(` signifies a function call
		// ExpressionLists are collected by default
		// `NoExpressionLists` ignores expression lists
		// If this flag is not set, then we will connect all expressions into a single token
		if (!IsFlagSet(LexerFlags::NoFunctionCalls) && _Input[i] == '('
			&& isalpha(_Input[_Index])) {
			
			if (!ExpressionLength(i, '(', ')') && IsFlagSet(LexerFlags::Verbose)) {
				std::cerr << "Warning: Expected `)` got EOL\n";
			}

			token = Token(*this, std::string_view(_Input + _Index, i - _Index), TokenType::FunctionCall);
		}
		else if (!IsFlagSet(LexerFlags::NoVectors) && _Input[_Index] == '<') {
			if (!ExpressionLength(i, '<', '>') && IsFlagSet(LexerFlags::Verbose)) {
				std::cerr << "Warning: Expected `>` got EOL\n";
			}

			token = Token(*this, std::string_view(_Input + _Index, i - _Index), TokenType::Vector);
		}
		else {
			token = Token(*this, std::string_view(_Input + _Index, i - _Index), type);
		}

		_Index = i;
	}
	else if (type == TokenType::Operator) {
		token = Token(*this, std::string_view(_Input + _Index++, 1), TokenType::Operator);
	}
	return token;
}

Token Lexer::Peek() {
	auto type = TokenType(_Input[_Index]);
	Token token;
	size_t i;
	for (i = _Index + 1; i < _Size && _Input[i]; i++) {
		if (type != TokenType(_Input[i])) {
			break;
		}
	}
	if (i - _Index - 1) {
		token = Token(*this, std::string_view(_Input + _Index + 1, i - _Index - 1), type);
	}
	return token;
}

size_t Lexer::ReadFunctionName(std::string& Name) {
	if (!IsFlagSet(LexerFlags::NoFunctionCalls)) {
		std::cerr << "Warning: ReadFunctionName called on expression list lexer\n";
		return 1;
	}

	Name = Read()._Str;
	return 0;
}

size_t Lexer::ReadFunctionName(std::string_view& Name) {
	if (!IsFlagSet(LexerFlags::NoFunctionCalls)) {
		std::cerr << "Warning: ReadFunctionName called on expression list lexer\n";
		return 1;
	}

	Name = Read()._Str;
	return 0;
}

size_t Lexer::ReadExpressionList(std::vector<std::string_view>& Arguments, 
	char BeginToken, char EndToken, char DelimiterToken, bool Clear) {
	if (Clear) {
		Arguments.clear();
	}

	if ((BeginToken == '(' && !IsFlagSet(LexerFlags::NoFunctionCalls)) || 
		(BeginToken == '<' && !IsFlagSet(LexerFlags::NoVectors))) {
		std::cerr << "Warning: ReadExpressionList on expression list lexer\n";
		return 1;
	}

	auto token = Read();

	if (token.Front() != BeginToken) {
		std::cerr << "Warning: ReadExpressionList on non-expression list\n";
		return 2;
	}

	static const char* _EmptyString = "";

	size_t i;
	for (i = _Index; i < _Size && _Input[i]; i++) {
		// Increment i so that _Index is after `)`
		if (_Input[i] == EndToken) {
			i++;
			break;
		}

		// Add a new argument after a comma or before adding chars
		if (_Input[i] == DelimiterToken) {
			Arguments.push_back(_EmptyString);
			continue;
		}

		// Hacky way of avoiding std::string
		if (!Arguments.size() || (!Clear && Arguments.size() == 1)) {
			Arguments.push_back(std::string_view(_Input + i, 1));
		}
		else if (Arguments.back().data() == _EmptyString) {
			Arguments.back() = std::string_view(_Input + i, 1);
		}
		else {
			Arguments.back() = std::string_view(Arguments.back().data(), Arguments.back().size() + 1);
		}
	}

	_Index = i;

	return 0;
}

bool Lexer::IsNegative() {
	if (_Prev.Type() != TokenType::Empty) {
		return Token::IsOperator(_Prev.Front());
	}
	return _Index == 1;
}

bool Lexer::ExpressionLength(size_t& i, char BeginToken, char EndToken) {
	size_t d = 1;

	// Don't count first `(`
	i++;

	while (i < _Size && _Input[i] && d) {
		if (_Input[i] == BeginToken)
			d++;
		if (_Input[i] == EndToken)
			d--;
		i++;
	}

	return d == 0;
}