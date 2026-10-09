#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include "TableDrawer.h"

const std::string EPSILON = "''";

struct Production {
    int id;
    std::string noTerminal;
    std::vector<std::string> derivacion;
};

struct LR1Item {
    int productionId;
    int dot;
    std::string lookahead;
};

struct Transition {
    int from;
    std::string symbol;
    int to;
};

struct Conflicto {
    int estado;
    std::string simbolo;
    std::string accionExistente;
    std::string accionNueva;
    std::string tipo;
};

using ItemSet = std::vector<LR1Item>;

class Parser {
private:
    std::filesystem::path baseDir;
    std::string buffer;

    std::unordered_map<std::string, std::vector<Production>> grammar;
    std::vector<Production> productions;

    std::unordered_set<std::string> noTerminales;
    std::unordered_set<std::string> terminales;
    std::vector<std::string> ordenSimbolos;
    std::unordered_map<std::string, std::unordered_set<std::string>> firstSet;

    std::string startSymbol;
    std::string augmentedStart = "S'";

    std::vector<Transition> transitions;

public:
    std::vector<Conflicto> conflictos;

public:
    Parser(std::filesystem::path inBaseDir) : baseDir(inBaseDir) {};

    void limpiarGramatica() {
        buffer.clear();

        grammar.clear();
        productions.clear();

        noTerminales.clear();
        terminales.clear();
        ordenSimbolos.clear();
        firstSet.clear();

        startSymbol.clear();
        transitions.clear();
        conflictos.clear();

        std::cout << "Parser limpiado correctamente" << std::endl;
    }

    void PrintGrammar() {
        for (auto& gram : grammar) {
            for (auto& prod : gram.second) {

                std::cout << "[" << prod.id << "] ";
                std::cout << prod.noTerminal << " -> ";

                for (auto& simbolo : prod.derivacion) {
                    std::cout << simbolo << " ";
                }

                std::cout << std::endl;
            }
        }
    }

    void registrarConflicto(int estado, const std::string& simbolo, const std::string& existente, const std::string& nueva){
        std::string tipo;

        if (existente[0] == 's' && nueva[0] == 'r' || existente[0] == 'r' && nueva[0] == 's') {
            tipo = "SHIFT-REDUCE";
        }
        else if (existente[0] == 'r' && nueva[0] == 'r') {
            tipo = "REDUCE-REDUCE";
        }
        else {
            tipo = "OTRO";
        }

        conflictos.push_back({estado, simbolo, existente, nueva, tipo});
    }

    void PrintSymbols() {
        std::cout << std::endl;
        std::cout << "No terminales:";
        std::cout << std::endl;

        for (auto& simbolo : noTerminales) {
            std::cout << simbolo << std::endl;
        }

        std::cout << std::endl;
        std::cout << "Terminales:";
        std::cout << std::endl;

        for (auto& simbolo : terminales) {
            std::cout << simbolo << std::endl;
        }

        std::cout << std::endl << "First Symbol: " << startSymbol << std::endl;
    }

    void ProcessGrammar(const std::string& buff) {
        int size = static_cast<int>(buff.size());
        int iter = 0;

        while (iter < size) {
            std::string tmpBuffer = {};
            Production tmpProd = {};

            while (iter < size && buff[iter] != ' ') {
                tmpBuffer += buff[iter];
                iter++;
            }

            tmpProd.noTerminal = tmpBuffer;
            tmpBuffer = {};

            if (!tmpProd.noTerminal.empty()) {
                if (std::find(ordenSimbolos.begin(), ordenSimbolos.end(),tmpProd.noTerminal) == ordenSimbolos.end()) {
                    ordenSimbolos.push_back(tmpProd.noTerminal);
                }

                noTerminales.insert(tmpProd.noTerminal);
                if (startSymbol.empty()) {
                    startSymbol = tmpProd.noTerminal;
                }
            }

            iter++;

            while (iter < size && buff[iter] != '-') {
                iter++;
            }

            if (iter < size && buff[iter] == '-') {
                iter++;
            }

            if (iter < size && buff[iter] == '>') {
                iter++;
            }

            while (iter < size && buff[iter] == ' ') {
                iter++;
            }

            while (iter < size && buff[iter] != '\n') {
                if (buff[iter] == ' ') {

                    if (!tmpBuffer.empty()) {

                        if (tmpBuffer != EPSILON) {
                            tmpProd.derivacion.push_back(tmpBuffer);
                        }

                        if (tmpBuffer != EPSILON) {
                            if (std::find(ordenSimbolos.begin(), ordenSimbolos.end(),tmpBuffer) == ordenSimbolos.end()) {

                                ordenSimbolos.push_back(tmpBuffer);
                            }

                            noTerminales.insert(tmpBuffer);
                        }

                        tmpBuffer = {};
                    }

                    iter++;
                    continue;
                }

                tmpBuffer += buff[iter];
                iter++;
            }

            if (!tmpBuffer.empty()) {
                if (tmpBuffer != EPSILON) {
                    tmpProd.derivacion.push_back(tmpBuffer);
                }

                if (tmpBuffer != EPSILON) {
                    if (std::find(ordenSimbolos.begin(), ordenSimbolos.end(),tmpBuffer) == ordenSimbolos.end()) {
                        ordenSimbolos.push_back(tmpBuffer);
                    }

                    noTerminales.insert(tmpBuffer);
                }
            }

            iter++;
            if (!tmpProd.noTerminal.empty()) {
                tmpProd.id = static_cast<int>(productions.size());
                productions.push_back(tmpProd);
                grammar[tmpProd.noTerminal].push_back(tmpProd);
            }
        }

        std::unordered_set<std::string> verdaderosNoTerminales;

        for (auto it = grammar.begin();it != grammar.end();++it) {
            verdaderosNoTerminales.insert(it->first);
        }

        for (auto it = noTerminales.begin(); it != noTerminales.end();) {
            if (verdaderosNoTerminales.find(*it) == verdaderosNoTerminales.end()) {
                terminales.insert(*it);
                it = noTerminales.erase(it);
            }
            else {
                it++;
            }
        }

        terminales.insert("$");

        AddAugmentedProduction();

        PrintGrammar();
        PrintSymbols();
    }

    void AddAugmentedProduction() {
        if (startSymbol.empty()) {
            return;
        }

        Production augmented;

        augmented.id =static_cast<int>(productions.size());
        augmented.noTerminal = augmentedStart;
        augmented.derivacion.push_back(startSymbol);

        productions.push_back(augmented);

        grammar[augmentedStart].insert(grammar[augmentedStart].begin(),augmented);
        noTerminales.insert(augmentedStart);
    }

    void LoadFile(const std::filesystem::path& filePath) {
        limpiarGramatica();
        std::filesystem::path fullPath = filePath.is_absolute() ? filePath : (baseDir / filePath);

        if (!std::filesystem::exists(fullPath)) {
            std::cerr << "ERROR: File does not exists!: " << fullPath << std::endl;

            return;
        }

        std::string source{ fullPath.string() };
        std::ifstream file(source);

        if (!file) {
            std::cerr << "PARSER: File not found! " << source << std::endl;

            return;
        }

        buffer.clear();

        std::stringstream bufferSS;
        bufferSS << file.rdbuf();
        buffer = bufferSS.str();

        file.close();
        ProcessGrammar(buffer);
    }

    bool HasEpsilon(const std::string& symbol) {
        return firstSet[symbol].find(EPSILON) != firstSet[symbol].end();
    }

    bool AddFirst(const std::string& A,const std::unordered_set<std::string>& nuevos) {
        bool changed = false;
        for (auto it = nuevos.begin(); it != nuevos.end() ;it++) {

            if (firstSet[A].find(*it) == firstSet[A].end()) {

                firstSet[A].insert(*it);
                changed = true;
            }
        }

        return changed;
    }

    void InsertWithoutEps(const std::string& A,std::unordered_set<std::string>& nuevos) {
        for (auto p : firstSet[A]) {
            if (p != EPSILON) {
                nuevos.insert(p);
            }
        }
    }

    void first() {
        for (auto t : terminales) {
            firstSet[t].insert(t);
        }

        firstSet[EPSILON].insert(EPSILON);

        for (auto nt : noTerminales) {
            firstSet[nt];
        }

        bool cambio = true;
        while (cambio) {
            cambio = false;

            for (auto gram : grammar) {
                std::string NT = gram.first;

                for (auto prod : gram.second) {
                    int size =static_cast<int>(prod.derivacion.size());
                    std::unordered_set<std::string> rhs;
                    bool todosEpsilon = true;
                    for (int i = 0; i < size; i++) {
                        std::string X =prod.derivacion[i];

                        InsertWithoutEps(X, rhs);

                        if (!HasEpsilon(X)) {
                            todosEpsilon = false;
                            break;
                        }
                    }

                    if (todosEpsilon) {
                        rhs.insert(EPSILON);
                    }

                    if (AddFirst(NT, rhs)) {
                        cambio = true;
                    }
                }
            }
        }
    }

    std::unordered_set<std::string> firstCadena(const std::vector<std::string>& cadena) {
        std::unordered_set<std::string> resultado;
        bool todosEpsilon = true;

        for (auto simbolo : cadena) {
            InsertWithoutEps(simbolo,resultado);

            if (!HasEpsilon(simbolo)) {
                todosEpsilon = false;
                break;
            }
        }

        if (todosEpsilon) {
            resultado.insert(EPSILON);
        }

        return resultado;
    }

    bool SameItem(const LR1Item& a,const LR1Item& b) {
        return a.productionId == b.productionId &&a.dot == b.dot &&a.lookahead == b.lookahead;
    }

    bool AddItem(ItemSet& items,const LR1Item& nuevo) {
        for (auto actual : items) {
            if (SameItem(actual, nuevo)) {
                return false;
            }
        }

        items.push_back(nuevo);

        return true;
    }

    ItemSet closure(ItemSet& item) {
        bool cambio = true;
        while (cambio) {
            cambio = false;
            for (int i = 0; i < item.size(); i++) {
                LR1Item p = item[i];

                Production prod = productions[p.productionId];
                if (p.dot >= prod.derivacion.size()) {
                    continue;
                }

                std::vector<std::string> cadena;
                std::string symbol = prod.derivacion[p.dot];

                if (noTerminales.find(symbol) != noTerminales.end()) {
                    for (int j = p.dot + 1; j < prod.derivacion.size(); j++) {
                        cadena.push_back(prod.derivacion[j]);
                    }
                    cadena.push_back(p.lookahead);
                }

                std::unordered_set<std::string> primero = firstCadena(cadena);
                for (auto &pp : grammar[symbol]) {
                    for (auto prim : primero) {
                        LR1Item tmpItem = {pp.id,0,prim};

                        if (AddItem(item, tmpItem)) {
                            cambio = true;
                        }
                    }
                    
                }

                
            }

        }
        return item;
    }

    ItemSet gotto(ItemSet& item, const std::string& X) {
        ItemSet new_items = {};

        for (LR1Item itm : item) {
            Production prod = productions[itm.productionId];

            if (itm.dot >= prod.derivacion.size()) {
                continue;
            }

            if (prod.derivacion[itm.dot] == X) {
                itm.dot++;
                AddItem(new_items, itm);
            }

        }
        return closure(new_items);
    }

    void PrintItems(ItemSet& items) {
        for (auto& item : items) {
            Production prod = productions[item.productionId];
            std::cout << "[" << prod.noTerminal << " -> ";

            for (int i = 0; i < prod.derivacion.size(); i++) {
                if (i == item.dot) {
                    std::cout << ".";
                }

                std::cout << prod.derivacion[i] << " ";
            }

            if (item.dot == prod.derivacion.size()) {
                std::cout << ".";
            }

            std::cout << ", " << item.lookahead << "]" << std::endl;
        }
    }
    void PrintFirstSet() {
        std::cout << std::endl;
        std::cout << "FIRST:" << std::endl;

        for (auto it = firstSet.begin(); it != firstSet.end(); ++it) {
            std::cout << "FIRST(" << it->first << ") = { ";

            for (auto simbolo : it->second) {
                std::cout << simbolo << " ";
            }

            std::cout << "}" << std::endl;
        }
    }

    bool CheckNew(const ItemSet& a, const ItemSet& b) {
        if (a.size() != b.size()) {
            return true;
        }

        for (auto& itemA : a) {
            bool encontrado = false;
            for (auto& itemB : b) {
                if (SameItem(itemA, itemB)) {
                    encontrado = true;
                    break;
                }
            }

            if (!encontrado) {
                return true;
            }
        }

        return false;
    }

    std::vector<ItemSet> coleccionCanonica() {
        transitions.clear();
        if ((int)firstSet.size() <= 0) {
            first();
        }

        std::vector<ItemSet> CCs;
        LR1Item itemInicial = { static_cast<int>(productions.size()) - 1, 0, "$" };
        ItemSet estadoInicial = closure(ItemSet{ itemInicial });

        CCs.push_back(estadoInicial);
        std::vector<std::string> simbolos = ordenSimbolos;

        for (int i = 0; i < CCs.size(); i++) {
            for (auto X : simbolos) {
                ItemSet new_CC = gotto(CCs[i], X);

                if (new_CC.empty()) {
                    continue;
                }

                int destino = -1;

                for (int j = 0; j < CCs.size(); j++) {
                    if (!CheckNew(new_CC, CCs[j])) {
                        destino = j;
                        break;
                    }
                }

                if (destino == -1) {
                    CCs.push_back(new_CC);
                    destino = CCs.size() - 1;
                }

                Transition transition;
                transition.from = i;
                transition.symbol = X;
                transition.to = destino;

                transitions.push_back(transition);
            }
        }

        return CCs;
    }

    void PrintTransitions() {
        std::cout << std::endl;
        std::cout << "TRANSICIONES:" << std::endl;

        for (auto t : transitions) {
            std::cout << "CC" << t.from << " --" << t.symbol << "--> CC" << t.to << std::endl;
        }
    }

    void agregarAccion(std::map<std::string, std::string>& actionMap,int estado,const std::string& simbolo,const std::string& accion){
        auto it = actionMap.find(simbolo);

        if (it == actionMap.end()) {
            actionMap[simbolo] = accion;
        }
        else if (it->second != accion) {
            actionMap[simbolo] += "/" + accion;
            registrarConflicto(estado, simbolo, it->second, accion);
        }
    }

    void llenarTablas(const std::string& nombreArchivo = "tabla_parseo.html") {
        std::vector<StateRow> table;
        std::vector<ItemSet> CCs = coleccionCanonica();

        for (int i = 0; i < CCs.size(); i++) {
            std::map<std::string, std::string> actionMap;
            std::map<std::string, std::string> gotoMap;

            for (auto t : transitions) {
                if (t.from != i) {
                    continue;
                }

                if (terminales.find(t.symbol) != terminales.end()) {
                    agregarAccion(actionMap, i, t.symbol, "s" + std::to_string(t.to));
                }
                else if (noTerminales.find(t.symbol) != noTerminales.end()) {
                    gotoMap[t.symbol] = std::to_string(t.to);
                }
            }

            for (LR1Item cc : CCs[i]) {
                Production prod = productions[cc.productionId];

                if (cc.dot >= (int)prod.derivacion.size()) {
                    if (prod.noTerminal == augmentedStart && cc.lookahead == "$") {
                        agregarAccion(actionMap, i, "$", "acc");
                    } else {
                        agregarAccion(actionMap, i, cc.lookahead, "r" + std::to_string(cc.productionId + 1));
                    }
                }
            }

            table.push_back(StateRow{i,actionMap,gotoMap});
        }

        TableDrawer tables{ baseDir };
        std::vector<std::string> terminalesVec(terminales.begin(), terminales.end());
        std::vector<std::string> noTerminalesVec(noTerminales.begin(), noTerminales.end());
        tables.saveToFile(nombreArchivo, terminalesVec, noTerminalesVec, table);
    }


    void probarCarpeta(const std::string& relativeFolderPath) {
        std::filesystem::path fullFolder = baseDir / relativeFolderPath;

        std::cout << "========================================" << std::endl;
        std::cout << "INICIANDO PRUEBAS DE GRAMATICAS " << std::endl;
        std::cout << "Carpeta: " << fullFolder << std::endl;
        std::cout << "========================================" << std::endl;

        if (!std::filesystem::exists(fullFolder) || !std::filesystem::is_directory(fullFolder)) {
            std::cerr << "ERROR: carpeta invalida: " << fullFolder << std::endl;
            return;
        }

        std::vector<std::filesystem::path> archivos;

        for (const auto& entry :
            std::filesystem::directory_iterator(fullFolder)) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                archivos.push_back(entry.path());
            }
        }

        std::sort(archivos.begin(), archivos.end());
        if (archivos.empty()) {
            std::cout << "No se encontraron archivos .txt" << std::endl;
            return;
        }

        int pruebasCorrectas = 0;
        int pruebasConConflictos = 0;
        int pruebasFallidas = 0;

        for (const auto& archivo : archivos) {
            std::cout << "----------------------------------------" << std::endl;
            std::cout << "GRAMATICA: " << archivo.filename().string() << std::endl;
            std::cout << "----------------------------------------" << std::endl;

            Parser prueba(baseDir);
            prueba.LoadFile(archivo);

            if (prueba.productions.empty() ||
                prueba.startSymbol.empty()) {
                std::cout << "ERROR: No se pudo cargar la gramatica" << std::endl;
                pruebasFallidas++;
                continue;
            }

            prueba.first();

            std::vector<ItemSet> CCs = prueba.coleccionCanonica();
            std::string nombreTabla = "tabla_parseo_" + archivo.stem().string() + ".html";

            prueba.llenarTablas(nombreTabla);

            std::cout << "Estados generados: " << CCs.size() << std::endl;

            if (prueba.conflictos.empty()) {
                std::cout << "RESULTADO: SIN CONFLICTOS" << std::endl;
                pruebasCorrectas++;
            }
            else {
                std::cout << "RESULTADO: CON CONFLICTOS" << std::endl;
                std::cout << "Cantidad: " << prueba.conflictos.size() << std::endl;

                for (const auto& c : prueba.conflictos) {
                    std::cout << "  Tipo: " << c.tipo << " | Estado: " << c.estado << " | Simbolo: " << c.simbolo << " | Existente: " << c.accionExistente << " | Nueva: " << c.accionNueva << std::endl;
                }

                pruebasConConflictos++;
            }

            std::cout << "Tabla generada: " << nombreTabla << std::endl;
        }

        std::cout << "========================================" << std::endl;
        std::cout << "RESUMEN DE PRUEBAS" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "Total de archivos: " << archivos.size() << std::endl;
        std::cout << "Sin conflictos: " << pruebasCorrectas << std::endl;
        std::cout << "Con conflictos: " << pruebasConConflictos << std::endl;
        std::cout << "Fallidas al cargar: " << pruebasFallidas << std::endl;
    }


};