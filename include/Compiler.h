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

	void load_grammar(const std::string& tableOutput = "grammar_table.html")
	{
		
		std::filesystem::path chocopyPath = PROJECT_PATH / "grammar/chocopy.txt";
		parser.LoadFile(chocopyPath);

		parser.first();
		parser.follow();

		parser.llenarTablas(tableOutput);
	}

	void compile(const std::string& sourcePath, const std::vector<std::string>& panicNTs, const std::string &fileOutput)
	{
		scanner.loadOneFile(PROJECT_PATH / sourcePath);
		parser.initializePanicMode(panicNTs);

		try
		{
			parser.followingStream(scanner, fileOutput);
		}
		catch (const std::exception& e)
		{
			std::cout << "Error: " << e.what() << std::endl;
		}

		std::cout << "TOTAL SCANNER ERRORS: " << scanner.getErrorCounter() << "\n";
	}

private:
	std::filesystem::path PROJECT_PATH;

	Scanner scanner;
	Parser parser;
};

#endif