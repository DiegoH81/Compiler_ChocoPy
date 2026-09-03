#include <iostream>
#include <string>
#include <filesystem>

#include "token.h"
#include "scanner.h"
#include "compiler.h"


int main()
{
	Token test(TokenType::IDENTIFIER, "TESTING", Position(1, 10));
	test.print();

	Scanner testScanner;

	//testScanner.loadOneFile("test/t1.txt");
	//testScanner.scanWholeThing();

	testScanner.bulkScan("test/");

	return 0;
}