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
	Operator,
	FunctionCall,
};

static constexpr enum class TokenType TokenType(char ch) {
	return IsOperator(ch) ? TokenType::Operator : TokenType::Literal;
}

class Token {
public:
	Token() : _Type(TokenType::Empty) {
	}

	Token(std::string_view Str) 
		: _Str(Str), _Type(!Front() ? TokenType::Empty : TokenType(Front())) {
	}

	inline char Front() const {
		return _Str.front();
	}

	inline enum class TokenType Type() const {
		return _Type;
	}

	std::string_view		_Str;
	enum class TokenType	_Type;
};

typedef double (*SyntaxCFunc)(const std::string& Name, const std::vector<double>& Arguments);

class SyntaxTree {
public:
	~SyntaxTree() {
		_Left.reset();
		_Right.reset();
	}
	
	SyntaxTree(const class Token& Token, std::shared_ptr<SyntaxTree> Left, std::shared_ptr<SyntaxTree> Right) {
		_Token = Token;
		_Left = Left;
		_Right = Right;
	}

	void Print(size_t Depth = 0) {
		for (size_t i = 0; i < Depth; i++)
			std::cout << '\t';

		std::cout << _Token._Str << '\n';

		if (_Left)
			_Left->Print(++Depth);

		if (_Right)
			_Right->Print(Depth);
	}

	double Eval() {
		if (_Token.Type() == TokenType::Operator && _Left && _Right) {
			switch (_Token.Front()) {
			case '^': {
				double left = _Left->Eval();
				if (round(left) == 0.0) {
					std::cerr << "Warning: 0^x\n";
					return 0.0;
				}
				return pow(left, _Right->Eval());
			}
			case '*':
				return _Left->Eval() * _Right->Eval();
			case '/':
				return _Left->Eval() / _Right->Eval();
			case '+':
				return _Left->Eval() + _Right->Eval();
			case '-':
				return _Left->Eval() - _Right->Eval();
			}
			return 0;
		}
		else if (_Token.Type() == TokenType::FunctionCall)
			return DoFunctionCall();
		else
			return GetValue();
	}

	Token _Token;
	std::shared_ptr<SyntaxTree> _Left, _Right;

	static std::unordered_map<std::string, double>		_Globals;
	static std::unordered_map<std::string, SyntaxCFunc>	_CFuncs;

private:
	double GetValue() {
		if (!_Token.Front())
			return 0.0;

		if (!isalpha(_Token.Front())) {
			double value = 0.0;
			std::from_chars(_Token._Str.data(), _Token._Str.data() + _Token._Str.size(), value);
			return value;
		}

		std::string varName(_Token._Str);
		
		auto it = _Globals.find(varName);
		
		if (it == _Globals.end()) {
			std::cerr << "Warning: Unknown variable " << varName << '\n';
			return 0.0;
		}

		return it->second;
	}
	
	// Requires lexer
	double DoFunctionCall();
};

enum class LexerFlags : unsigned char {
	Normal,
	NoFunctions = 1,
};

class Lexer {
public:
	friend class SyntaxTree;

	Lexer() : _Input(NULL), _Index(0), _Size(0), _Flags(LexerFlags::Normal) {
	}

	size_t InfixToPostfix(std::vector<Token>& Postfix) {
		if (!_Size) {
			std::cerr << "Warning: InfixToPostfix on empty string\n";
			return 1;
		}

		Postfix.clear();

		std::stack<Token> stack;
		Token token;
		while (Advance()) {
			token = Read();
			
			if (token.Type() == TokenType::Empty)
				continue;

			if (token.Type() != TokenType::Operator) {
				Postfix.push_back(token);
			}
			else {
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
						std::cerr << "Warning: Expected `)`\n";
						return 2;
					}
					stack.pop();
				}
			}
		}

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
		// This would fault if `Postfix` had no tokens
		if (!Postfix.size()) {
			std::cerr << "Warning: PostfixToSyntaxTree empty token vector\n";
			return nullptr;
		}

		std::stack<std::shared_ptr<SyntaxTree>> tree;

		for (const auto& Tok : Postfix) {
			if (Tok.Type() == TokenType::Operator) {
				std::shared_ptr<SyntaxTree> Right = tree.top();
				tree.pop();

				std::shared_ptr<SyntaxTree> Left = tree.top();
				tree.pop();

				tree.push(std::make_shared<SyntaxTree>(Tok, Left, Right));
			}
			else if (Tok.Type() != TokenType::Empty)
				tree.push(std::make_shared<SyntaxTree>(Tok, nullptr, nullptr));
		}

		std::shared_ptr<SyntaxTree> Head = tree.top();
		tree.pop();

		return Head;
	}

	bool Advance() {
		// March until we encounter a non-blank character
		for (; _Index < _Size && _Input[_Index] && isblank(_Input[_Index]); _Index++);
		return _Index < _Size;
	}

	Token Read() {
		auto type = TokenType(_Input[_Index]);
		Token token;
		if (type == TokenType::Literal) {
			size_t i;

			for (i = _Index + 1; i < _Size && _Input[i]; i++) {
				if (type != TokenType(_Input[i]))
					break;
			}

			// A literal alphabetical followed by a `(` signifies a function call
			if (!IsFlagSet(LexerFlags::NoFunctions) && _Input[i] == '(' && isalpha(_Input[_Index])) {
				if (!FunctionReferenceCallLength(i)) {
					std::cerr << "Warning: Expected `)` got EOL\n";
				}

				token = Token(std::string_view(_Input + _Index, i - _Index));
				token._Type = TokenType::FunctionCall;
			}
			else {
				token = Token(std::string_view(_Input + _Index, i - _Index));
			}
			
			_Index = i;
		}
		else if (type == TokenType::Operator) {
			token = Token(std::string_view(_Input + _Index++, 1));
		}
		return token;
	}

	size_t ReadFunctionName(std::string& Name) {
		if (!IsFlagSet(LexerFlags::NoFunctions)) {
			std::cerr << "Warning: ReadFunctionName called on function lexer\n";
			return 1;
		}
		
		Name = Read()._Str;
		return 0;
	}

	size_t ReadFunctionCallArguments(std::vector<std::string_view>& Arguments) {
		Arguments.clear();
		
		if (!IsFlagSet(LexerFlags::NoFunctions)) {
			std::cerr << "Warning: ReadFunctionCallArguments on function lexer\n";
			return 1;
		}

		auto token = Read();

		if (token.Front() != '(') {
			std::cerr << "Warning: ReadFunctionCallArguments on non-function\n";
			return 2;
		}

		static const char* _EmptyString = "";

		size_t i;
		for (i = _Index; i < _Size && _Input[i]; i++) {
			// Increment i so that _Index is after `)`
			if (_Input[i] == ')') {
				i++;
				break;
			}

			// Add a new argument after a comma or before adding chars
			if (_Input[i] == ',') {
				Arguments.push_back(_EmptyString);
				continue;
			}

			// Hacky way of avoiding std::string
			if (!Arguments.size()) {
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

	void Reset() {
		_Index = 0;
	}

	void SetString(const char* Str, size_t Size) {
		_Input = Str;
		_Index = 0;
		_Size = Size;
	}

	void SetIndex(size_t Index) {
		_Index = Index;
	}

	void SetFlags(LexerFlags Flags) {
		_Flags = Flags;
	}

	bool IsFlagSet(LexerFlags Flags) const {
		return ((unsigned char)_Flags & (unsigned char)Flags) != 0;
	}

private:
	const char *_Input;
	size_t		_Index;
	size_t		_Size;
	LexerFlags _Flags;
	
	// Count the call token length following a function name
	bool FunctionReferenceCallLength(size_t& i) {
		size_t d = 1;
		
		// Don't count first `(`
		i++;

		while (i < _Size && _Input[i] && d) {
			if (_Input[i] == '(')
				d++;
			if (_Input[i] == ')')
				d--;
			i++;
		}

		return d == 0;
	}
};

double SyntaxTree::DoFunctionCall() {
	Lexer lexer;
	lexer.SetString(_Token._Str.data(), _Token._Str.length());
	lexer.SetFlags(LexerFlags::NoFunctions);

	std::string funcName;

	if (lexer.ReadFunctionName(funcName)) {
		return 0.0;
	}

	auto func = _CFuncs.find(funcName);

	if (func == _CFuncs.end()) {
		std::cerr << "Warning: Unknown function call " << funcName << '\n';
		return 0.0;
	}

	std::vector<std::string_view> strArgs;

	if (lexer.ReadFunctionCallArguments(strArgs)) {
		return 0.0;
	}

	std::vector<double> args;
	std::vector<Token> postfix;

	// Allow for nested function calls
	lexer.SetFlags(LexerFlags::Normal);

	for (const auto& str : strArgs) {
		lexer.SetString(str.data(), str.length());
		
		// Warn & exit safely
		if (lexer.InfixToPostfix(postfix)) {
			return 0.0;
		}

		// Tree is a shared ptr and gets deleted
		auto tree = lexer.PostfixToSyntaxTree(postfix);
		
		if (tree)
			args.push_back(tree->Eval());
	}

	// Perform the function call to SyntaxCFunc
	return func->second(funcName, args);
}

std::unordered_map<std::string, double>	SyntaxTree::_Globals {
	{"pi", M_PI},
	{"e", M_E},
	{"G", 0.00000000006673},
	{"g", 9.81},
};

namespace SyntaxFuncs {
	static double cos(const std::string& Name, const std::vector<double>& Arguments) {
		if (Arguments.size() != 1) {
			std::cerr << "Warning: Call to cosine with incorrect argument(s)\n";
			return 0.0;
		}
		return ::cos(Arguments[0]);
	}
};

std::unordered_map<std::string, SyntaxCFunc> SyntaxTree::_CFuncs {
	{"cos", SyntaxFuncs::cos},
};

int main() {
	const char str[] = "5*cos(";
	Lexer lexer;
	std::vector<Token> postfix;
	lexer.SetString(str, sizeof(str));
	lexer.InfixToPostfix(postfix);
	auto tree = lexer.PostfixToSyntaxTree(postfix);
	tree->Print();
	std::cout << tree->Eval() << std::endl;
}