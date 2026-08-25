#include <iostream>
#include <string>

#include "token.h"

int main()
{
	Token test(TokenType::IDENTIFIER, "TESTING");
	test.print();

	return 0;
}