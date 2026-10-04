#ifndef COMPILER_H
#define COMPILER_H

#include <string>
#include "Scanner.h"

class Compiler
{
public:
	Compiler():
		PROJECT_PATH(CHOCOPY_ROOT), scanner(PROJECT_PATH)
	{}

	void compile(const std::string& sourcePath)
	{
		scanner.loadOneFile(sourcePath);
		scanner.scanWholeThing();

	}

private:
	std::filesystem::path PROJECT_PATH;

	Scanner scanner;
};

#endif