# Compiler ChocoPy

Final project for Compilers Course.

Made by:
- Alvarez Puma, Alan Patrizio
- Hidalgo Machaca, Diego Alejandro
- Valenzuela Calderón, Luigi Yamil

---

## Overview

This is a front end for [ChocoPy](https://chocopy.org/), a statically typed subset of Python, written in C++17. It covers the analysis phases of a compiler:

1. **Lexical analysis**: a hand-written scanner turns source text into tokens.
2. **Syntax analysis**: a canonical **LR(1)** parser. It builds its ACTION/GOTO tables at startup from a grammar file, and it recovers from errors with panic mode.
3. **Semantic analysis**: runs during parsing on every reduce. It builds scoped symbol tables, checks declaration and scoping rules, checks basic types and builds an Abstract Syntax Tree.

Every phase can write an **HTML report** (parsing table, parse trace, symbol table, AST), so you can inspect the results in a browser.

---

## Requirements

- CMake **3.20** or newer
- A C++17 compiler (MSVC / Visual Studio 2019+, GCC 9+ or Clang 10+)
- An internet connection the first time you configure. CMake downloads these dependencies with `FetchContent`:
  - [nlohmann/json](https://github.com/nlohmann/json) v3.11.3
  - [inja](https://github.com/pantor/inja) v3.4.0 (template engine for the HTML reports)

---

## Building

From the project root:

```bash
cmake -S . -B build
cmake --build build
```

This creates the `chocopy` executable:

You can also open the folder directly in Visual Studio or CLion, since both support CMake projects.

> The macro `CHOCOPY_ROOT` is set at compile time to the project directory. The compiler finds the grammar, templates and tests through it, and it writes the reports there too, so it works from any working directory.

---

## Running

```bash
./build/chocopy          # or build\Debug\chocopy.exe on Windows
```

`src/main.cpp` does:

1. Loads `grammar/chocopy.txt`, computes FIRST/FOLLOW sets, builds the LR(1) canonical collection and tables, writes `tables/grammar_table.html` and prints the number of LR(1) conflicts.
2. Compiles a set of test files (`test/SemanticTest`, `test/TypeTest`, `test/ASTTest`). For each one it writes the parse trace, symbol table and, for the AST tests, the AST.
3. Prints an error summary and the result (`programa aceptado sin errores` or `compilacion con errores`) for each file.

To compile a different file, edit `src/main.cpp`:

```cpp
Compiler compiler;
compiler.load_grammar();                          // -> tables/grammar_table.html

compiler.setAstDebug(true);                       // optional: print pre/post-order traversals
compiler.compile("test/ASTTest/a1.txt",           // source file (relative to project root)
                 { "stmt", "decl_item" },         // non-terminals used for panic-mode recovery
                 "trace_a1.html",                 // -> traces/trace_a1.html
                 "symbols_a1.html",               // -> tables/symbols_a1.html
                 "ast_a1.html");                  // -> tables/ast_a1.html (optional)
```

### Generated output

| Folder    | File                                   | Content                                                                             |
|-----------|----------------------------------------|-------------------------------------------------------------------------------------|
| `tables/` | `grammar_table.html`                   | LR(1) ACTION/GOTO table                                                             |
| `traces/` | `tracer_*.html`                        | Step-by-step parse trace (stack, input, action). Error and discarded states are shown in red |
| `tables/` | `symbols_*.html`                       | Symbol table, grouped by scope (global, class, function)                            |
| `tables/` | `ast_*.html`                           | AST viewer                                                                          |

---


## Tests

All test inputs are ChocoPy programs in plain `.txt` files under `test/`:

| Folder               | Files            | What it tests |
|----------------------|------------------|---------------|
| `test/ScannerTest/`  | `t1` – `t10`     | **Lexer**: tokens, indentation/dedentation, integer and string literals, and lexical errors (invalid characters, tabs, bad escapes, unclosed strings, out-of-range integers, etc.) |
| `test/ParserTest/`   | `t1` – `t12`, `ChocopyGrammar.txt` | **Parser**: valid and invalid programs for the LR(1) parser and panic-mode recovery. `ChocopyGrammar.txt` is an earlier version of the grammar |
| `test/SemanticTest/` | `s1` – `s7`      | **Semantic rules**: scopes, redeclarations, `global`/`nonlocal`, inheritance, method `self` parameter, `return` outside a function, undefined names. `s1` is a valid program with no errors |
| `test/TypeTest/`     | `t1` – `t3`      | **Type checking**: operators on `int`/`str`/`bool`, conditions, assignments and initializations with incompatible types |
| `test/ASTTest/`      | `a1` – `a3`      | **AST construction**: classes, methods, control flow, expressions; generates the AST HTML viewer and prints traversals |

By default `main.cpp` runs `SemanticTest/s7`, `TypeTest/t1` and `ASTTest/a1`–`a3`. Change the loop ranges to run others. The scanner and parser folders can also be run in bulk with the helpers `Scanner::bulkScan(folder)` and `Parser::probarCarpeta(folder)`.