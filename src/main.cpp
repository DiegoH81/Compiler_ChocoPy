#include <iostream>
#include <string>
#include <filesystem>

#include "Compiler.h"
#include "HashTable.h"
#include "SymbolTable.h"

int main()
{
    Compiler compiladorsin;

    compiladorsin.load_grammar();

    for (int i = 10; i <= 10; i++)
    {
        std::string archivo = "test\\ScannerTest\\t" + std::to_string(i) + ".txt";
        std::string tracer = "tracer" + std::to_string(i) + ".html";

        compiladorsin.compile(archivo, { "stmt", "decl_item" }, tracer);
    }

    return 0;
}