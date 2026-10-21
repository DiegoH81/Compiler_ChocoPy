#include <iostream>
#include <string>
#include <filesystem>

#include "token.h"
#include "scanner.h"
#include "compiler.h"
#include "TableDrawer.h"
#include "FileManager.h"

int main()
{
    std::filesystem::path baseDir = std::filesystem::path(CHOCOPY_ROOT);

    Parser testParser(baseDir);

    std::filesystem::path testi = baseDir / "grammar/chocopy.txt";
    testParser.LoadFile(testi);

    testParser.first();
    testParser.PrintFirstSet();

    testParser.follow();
    testParser.PrintFollowSet();

    testParser.PrintGrammar();

    testParser.llenarTablas();

    std::cout << "Probando parser LR(1)" << std::endl;
    testParser.initializePanicMode({ "stmt", "decl_item" });
    std::string input = "def ID ( ID : ID ) : NEWLINE INDENT pass NEWLINE DEDENT while ID < INTEGER : NEWLINE INDENT pass NEWLINE DEDENT";

    try
    {
        testParser.following(input);
    }
    catch (const std::exception& e)
    {
        std::cout << "Error: " << e.what() << std::endl;
    }

    return 0;
}