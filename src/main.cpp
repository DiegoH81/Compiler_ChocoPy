#include <iostream>
#include <string>
#include <filesystem>

#include "Compiler.h"

int main()
{
    Compiler compiladorsin;
    
    compiladorsin.load_grammar();
    compiladorsin.compile("test\\ScannerTest\\t1.txt", { "stmt", "decl_item" }, "tracer.html");
    

    return 0;
}