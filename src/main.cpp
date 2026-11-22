#include <iostream>
#include <string>
#include <filesystem>

#include "Compiler.h"

int main()
{
    Compiler compiladorsin;

    compiladorsin.load_grammar();

    for (int i = 1; i <= 7; i++)
    {
        std::string archivo = "test\\SemanticTest\\s" + std::to_string(i) + ".txt";
        std::string tracer = "tracer_s" + std::to_string(i) + ".html";
        std::string symbols = "symbols_s" + std::to_string(i) + ".html";

        compiladorsin.compile(archivo, { "stmt", "decl_item" }, tracer, symbols);
    }

    return 0;
}
