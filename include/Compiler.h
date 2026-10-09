#ifndef COMPILER_H
#define COMPILER_H

#include <string>
#include "Scanner.h"
#include "Parser.h"

class Compiler
{
public:
	Compiler():
		PROJECT_PATH(CHOCOPY_ROOT), scanner(PROJECT_PATH), parser(PROJECT_PATH)
	{}

	void compile(const std::string& sourcePath)
	{
		scanner.loadOneFile(sourcePath);
		scanner.scanWholeThing();

	}

private:
	std::filesystem::path PROJECT_PATH;

	Scanner scanner;
	Parser parser;
};

#endif