#include "stdafx.h"
#include "syntaxtree.h"
#include "token.h"
#include "lexer.h"

static TokenValue EvalLiteralExp(TokenValue& Left, TokenValue& Right) {
	double left = Left.GetNumber();
	
	if (round(left) == 0.0) {
		std::cerr << "Warning: Exponential base zero\n";
		return 0.0;
	}

	return pow(left, Right.GetNumber());
}

static TokenValue EvalLiteralDiv(TokenValue& Left, TokenValue& Right) {
	double right = Right.GetNumber();
	
	if (round(right) == 0.0) {
		std::cerr << "Warning: Divide by zero\n";
		return 0.0;
	}

	return Left.GetNumber() / right;
}

//
//  If it's an operator, evaluate left and right subtrees, and
// return result. If not, return it's "useful" value.
//	E.g. Resolved variable, performed function call return, vector, 
// or in-text numerical value.
//
TokenValue SyntaxTree::Eval() const {
	if (_Token.Type() == TokenType::Operator && _Left && _Right) {
		// Eval first so that the types are correct after function calls etc
		TokenValue Left = _Left->Eval(), Right = _Right->Eval();
		// Literal Literal
		switch (_Token.Front()) {
		case '^': return Left ^ Right;
		case '*': return Left * Right;
		case '/': return Left / Right;
		case '+': return Left + Right;
		case '-': return Left - Right;
		}
		return 0.0;
	}
	else if (_Token.Type() == TokenType::FunctionCall)
		return DoFunctionCall();
	else
		return GetValue();
}

TokenValue SyntaxTree::GetValue() const {
	if (!_Token.Front()) {
		return 0.0;
	}

	// Literal number
	if (_Token.Type() == TokenType::Number || _Token.Type() == TokenType::Vector) {
		return _Token._Value;
	}

	// Literal variable reference to number
	std::string varName(_Token._Str);

	auto it = _Globals.find(varName);

	if (it == _Globals.end()) {
		//std::cerr << "Warning: Unknown variable " << varName << '\n';
		return 0.0;
	}

	return TokenValue(it->second, _Token.Negative());
}

TokenValue SyntaxTree::DoFunctionCall() const {
	static Lexer lexer;

	// No function name
	if (_Token._Value._Call.size() < 1) {
		std::cerr << "Error: No function name in function call\n";
		return 0.0;
	}

	std::vector<TokenValue> args;

	// Calculate the comma-separated expressions (ExprList) into args
	if (EvalExprList(lexer, _Token._Value._Call, 1, false, args)) {
		std::cerr << "Error: EvalExprList failed in function call\n";
		return 0.0;
	}

	// Literal variable reference to number
	std::string funcName(_Token._Value._Call[0]);

	auto it = _CFuncs.find(funcName);

	if (it == _CFuncs.end()) {
		std::cerr << "Warning: Unknown function " << funcName << '\n';
		return 0.0;
	}

	return TokenValue(it->second(funcName, args), _Token.Negative());
}