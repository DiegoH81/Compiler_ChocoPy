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

    //Scanner testScanner(baseDir);
    //testScanner.bulkScan("test/ScannerTest");

    Parser testParser(baseDir);
    //testParser.probarCarpeta("test/ParserTest");

    std::filesystem::path testi = baseDir / "test/ParserTest/ChocopyGrammar.txt";
    testParser.LoadFile(testi);

    testParser.first();
    testParser.PrintFirstSet();

    testParser.follow();
    testParser.PrintFollowSet();

    testParser.PrintGrammar();


    /*std::vector<ItemSet> CCs = testParser.coleccionCanonica();
    for (size_t i = 0; i < CCs.size(); ++i)
    {
        std::cout << "\nCC" << i << ":\n";
        testParser.PrintItems(CCs[i]);
    }

    testParser.PrintTransitions();*/

    testParser.llenarTablas();
    std::cout << "Probando parser LR(1)" << std::endl;

    std::string input = "def ID ( ID : ID ) : NEWLINE INDENT pass NEWLINE DEDENT while ID < INTEGER : NEWLINE INDENT pass NEWLINE DEDENT";

    try {
        testParser.following(input);
    }
    catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }

    return 0;
}