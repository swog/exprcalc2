#pragma once

namespace expr2 {
	class TokenReceiver {
	public:
		virtual void operator()(const std::smatch& match) = 0;
	};

	class TokenContainer : public TokenReceiver {
	public:
		virtual void operator()(const std::smatch& match) {
			_tokens.push_back(match.str());
		}

		std::vector<std::string> _tokens;
	};

	void Lineify(const char* str, TokenReceiver& receiver);
	void Lexer(const char* str, TokenReceiver& receiver);
}