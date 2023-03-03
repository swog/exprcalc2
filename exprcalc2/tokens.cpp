#include "stdafx.h"
#include "tokens.h"

namespace expr2 {

void Lineify(const char* str, TokenReceiver& receiver) {
	static const std::regex regex("([^;\\n\\r].*?)(?=[;\\n\\r]|$)");
	std::string _str(str);
	std::sregex_iterator it(_str.cbegin(), _str.cend(), regex), end = std::sregex_iterator();
	std::for_each<std::sregex_iterator, TokenReceiver&>(it, end, receiver);
}

void Lexer(const char* str, TokenReceiver& receiver) {
	static const std::regex regex("([\\d\\.]+)|(([^\\d\\w()\\s]+)|(\\w)+|\\(|\\))");
	std::string _str(str);
	std::sregex_iterator it(_str.cbegin(), _str.cend(), regex), end = std::sregex_iterator();
	std::for_each<std::sregex_iterator, TokenReceiver&>(it, end, receiver);
}

}