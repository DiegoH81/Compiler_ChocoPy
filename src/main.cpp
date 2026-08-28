#include <iostream>
#include <string>

#include "token.h"

int main()
{
	Token test(TokenType::IDENTIFIER, "TESTING", Position(1, 10));
	test.print();

	return 0;
}