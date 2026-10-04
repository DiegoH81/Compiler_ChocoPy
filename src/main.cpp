#include <iostream>
#include <string>
#include <filesystem>

#include "token.h"
#include "scanner.h"
#include "compiler.h"


int main()
{
	std::filesystem::path baseDir = std::filesystem::path(CHOCOPY_ROOT);

	Token test(TokenType::IDENTIFIER, "TESTING", Position(1, 10));
	std::cout << test << std::endl;

	Scanner testScanner(baseDir);

	//testScanner.loadOneFile("test/t1.txt");
	//testScanner.scanWholeThing();

	testScanner.bulkScan("test/");

	return 0;
}