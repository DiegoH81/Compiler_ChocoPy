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
    testParser.probarCarpeta("test/ParserTest");

	return 0;
}