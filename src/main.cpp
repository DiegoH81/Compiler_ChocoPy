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

    // Individual test
    std::cout << "\n\n\nTesting individual\n";
    std::filesystem::path testi = CHOCOPY_ROOT + std::string{ "/test/ParserTest/t12.txt" };
    testParser.LoadFile(testi);

    testParser.first();
    testParser.PrintFirstSet();
    std::vector<std::string> cadena = { "Pair", "$" };

    auto resultado = testParser.firstCadena(cadena);

    std::cout << "FIRST(Pair $): ";
    for (auto simbolo : resultado)
        std::cout << simbolo << " ";

    std::cout << std::endl;

    LR1Item inicial = {};

    inicial.productionId = 3;
    inicial.dot = 0;
    inicial.lookahead = "$";

    ItemSet items;
    items.push_back(inicial);

    items = testParser.closure(items);

    testParser.PrintItems(items);
    ItemSet result = testParser.gotto(items, "Pair");

    std::cout << std::endl << "GOTO(Pair):" << std::endl;
    testParser.PrintItems(result);

    std::vector<ItemSet> CCs = testParser.coleccionCanonica();
    for (int i = 0; i < CCs.size(); i++)
    {
        std::cout << std::endl << "CC" << i << ":" << std::endl;
        testParser.PrintItems(CCs[i]);
    }

    testParser.PrintTransitions();

    std::cout << "\nSaving .... \n";
    testParser.llenarTablas();

    if (!testParser.conflictos.empty())
    {
        std::cout << std::endl;
        std::cout << "ERROR: Se encontraron " << testParser.conflictos.size() << " conflictos en la tabla LR(1):";
        std::cout << std::endl;

        for (const auto& c : testParser.conflictos)
            std::cout << "Tipo: " << c.tipo << "Estado: " << c.estado << "Simbolo: " << c.simbolo <<
                         "Accion existente: " << c.accionExistente <<
                         "Accion nueva: " << c.accionNueva << "\n";
    }
    else
        std::cout << "Tabla LR(1) construida sin conflictos";

	return 0;
}