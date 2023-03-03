#include "stdafx.h"
#include "tokens.h"
#include "syntaxtree.h"

static constexpr char Prescedence(char ch) {
	switch (ch) {
	case '^':
		return 4;
	case '*':
	case '/':
		return 3;
	case '+':
	case '-':
		return 2;
	}
	return 1;
}

static constexpr bool IsOperator(char ch) {
	return ch == '^' || ch == '*' || ch == '/' || ch == '+' || ch == '-';
}

char *InfixToPostfix(char *str) {
	std::stack<char> operators;
	size_t j = 0;

	for (size_t i = 0; str[i]; i++) {
		if (IsOperator(str[i]))
			str[j++] = str[i];
		else if (str[i] == '(')
			operators.push('(');
		else if (str[i] == ')') {
			while (operators.size() && operators.top() != '(') {
				str[j++] = operators.top();
				operators.pop();
			}
			if (operators.size() && operators.top() != '(')
				return NULL;
			else if (operators.size())
				operators.pop();
		}
		else {
			while (operators.size() && Prescedence(str[i]) <= Prescedence(operators.top())) {
				str[j++] = operators.top();
				operators.pop();
			}
			operators.push(str[i]);
		}
	}

	while (operators.size()) {
		str[j++] = operators.top();
		operators.pop();
	}

	str[j++] = '\0';
	return str;
}

int main() {
	char str[] = "((p*q)+(p+q)*q)";
	InfixToPostfix(str);
	std::cout << str << '\n';
}