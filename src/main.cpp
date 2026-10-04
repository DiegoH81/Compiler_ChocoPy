#include <iostream>
#include <string>
#include <filesystem>

#include "token.h"
#include "scanner.h"
#include "compiler.h"
#include "TableDrawer.h"


int main()
{
	std::filesystem::path baseDir = std::filesystem::path(CHOCOPY_ROOT);

    TableDrawer drawer(baseDir);

    std::vector<std::string> terminals = { "id", "+", "*", "$" };
    std::vector<std::string> nonTerminals = { "E", "T" };

    std::vector<StateRow> rows;

    rows.push_back(StateRow(
        0,
        { {"id", "s5"} },
        { {"E", "1"}, {"T", "2"} }
    ));

    rows.push_back(StateRow(
        1,
        { {"Target", "s6"}, {"$", "acc"} },
        {}
    ));

    drawer.saveToFile("tabla_parseo.html", terminals, nonTerminals, rows);

    return 0;

	Token test(TokenType::IDENTIFIER, "TESTING", Position(1, 10));
	std::cout << test << std::endl;

	Scanner testScanner(baseDir);

	//testScanner.loadOneFile("test/t1.txt");
	//testScanner.scanWholeThing();

	testScanner.bulkScan("test/");

	return 0;
}