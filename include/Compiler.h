#ifndef COMPILER_H
#define COMPILER_H

#include <string>
#include "Scanner.h"

class Compiler
{
public:
	Compiler():
		scanner()
	{}

	void compile(const std::string& sourcePath)
	{
		scanner.loadOneFile(sourcePath);
		scanner.scanWholeThing();
	}

private:
	Scanner scanner;
};

#endif