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

	void load_grammar()
	{
		std::filesystem::path chocopyPath = PROJECT_PATH / "grammar/chocopy.txt";
		parser.LoadFile(chocopyPath);

		parser.first();
		parser.follow();

		parser.llenarTablas();
	}

	void compile(const std::string& sourcePath, const std::vector<std::string>& inSelected )
	{
		scanner.loadOneFile(sourcePath);
		//scanner.scanWholeThing();

		parser.initializePanicMode(inSelected);

		parser.following(inSelected);
	}

private:
	std::filesystem::path PROJECT_PATH;

	Scanner scanner;
	Parser parser;
};

#endif