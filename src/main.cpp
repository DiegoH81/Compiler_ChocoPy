#include <iostream>
#include <string>
#include <filesystem>

#include "token.h"
#include "scanner.h"

int main()
{
	Token test(TokenType::IDENTIFIER, "TESTING", Position(1, 10));
	test.print();

	Scanner test_scanner("test/t1.txt");

	return 0;
}