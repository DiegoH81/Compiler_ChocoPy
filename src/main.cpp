#include <iostream>
#include <string>
#include <filesystem>

#include "Compiler.h"

int main()
{
    Compiler compiladorsin;

    compiladorsin.load_grammar();

    for (int i = 7; i <= 7; i++)
    {
        std::string archivo = "test\\SemanticTest\\s" + std::to_string(i) + ".txt";
        std::string tracer = "tracer_s" + std::to_string(i) + ".html";
        std::string symbols = "symbols_s" + std::to_string(i) + ".html";

        compiladorsin.compile(archivo, { "stmt", "decl_item" }, tracer, symbols);
    }

    for (int i = 1; i <= 1; i++)
    {
        std::string archivo = "test\\TypeTest\\t" + std::to_string(i) + ".txt";
        std::string tracer = "tracer_t" + std::to_string(i) + ".html";
        std::string symbols = "symbols_t" + std::to_string(i) + ".html";

        compiladorsin.compile(archivo, { "stmt", "decl_item" }, tracer, symbols);
    }

    compiladorsin.setAstDebug(true);

    for (int i = 1; i <= 3; i++)
    {
        std::string archivo = "test\\ASTTest\\a" + std::to_string(i) + ".txt";
        std::string tracer = "tracer_a" + std::to_string(i) + ".html";
        std::string symbols = "symbols_a" + std::to_string(i) + ".html";
        std::string ast = "ast_a" + std::to_string(i) + ".html";

        compiladorsin.compile(archivo, { "stmt", "decl_item" }, tracer, symbols, ast);
    }

    compiladorsin.setAstDebug(false);

    return 0;
}
