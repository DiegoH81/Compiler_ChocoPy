#ifndef COMPILER_H
#define COMPILER_H

#include <string>
#include "Scanner.h"
#include "Parser.h"
#include "ErrorManager.h"
#include "SemanticAnalyzer.h"
#include "SymbolTableDrawer.h"
#include "ASTTraversal.h"

class Compiler
{
public:
	Compiler():
		PROJECT_PATH(CHOCOPY_ROOT), scanner(PROJECT_PATH), parser(PROJECT_PATH)
	{
		scanner.setErrorManager(&errors);
		parser.setErrorManager(&errors);
		semantic.setErrorManager(&errors);
		parser.setReduceListener(&semantic);
	}

	void load_grammar(const std::string& tableOutput = "grammar_table.html")
	{
		
		std::filesystem::path chocopyPath = PROJECT_PATH / "grammar/chocopy.txt";
		parser.LoadFile(chocopyPath);

		parser.first();
		parser.follow();

		parser.llenarTablas(tableOutput);
		std::cout << "Conflictos LR(1): " << parser.conflictos.size() << "\n";
	}

	void setAstDebug(bool activo)
	{
		astDebug = activo;
	}

	void compile(const std::string& sourcePath, const std::vector<std::string>& panicNTs, const std::string &fileOutput, const std::string& symbolsOutput)
	{
		errors.clear();
		semantic.reset();
		std::cout << "\nCompilando: " << sourcePath << "\n";

		// less output bois
		semantic.debug = false;

		scanner.loadOneFile(PROJECT_PATH / sourcePath);
		parser.initializePanicMode(panicNTs);

		bool reachedEnd = false;

		try
		{
			reachedEnd = parser.followingStream(scanner, fileOutput);
		}
		catch (const std::exception& e)
		{
			std::cout << "Error: " << e.what() << std::endl;
		}

		if (astDebug && semantic.getAst())
			ASTTraversal::printRecorridos(semantic.getAst(), std::cout);

		semantic.printSymbols();

		SymbolTableDrawer symbolDrawer(PROJECT_PATH);
		symbolDrawer.saveToFile(symbolsOutput, sourcePath, semantic.getSymbols());

		errors.printSummary();

		if (reachedEnd && !errors.hasErrors())
			std::cout << "RESULTADO: programa aceptado sin errores\n";
		else
			std::cout << "RESULTADO: compilacion con errores\n";
	}

private:
	std::filesystem::path PROJECT_PATH;

	Scanner scanner;
	Parser parser;
	ErrorManager errors;
	SemanticAnalyzer semantic;
	bool astDebug = false;
};

#endif