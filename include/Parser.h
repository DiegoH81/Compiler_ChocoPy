#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <stack>
#include <queue>
#include <filesystem>
#include <algorithm>
#include <map>
#include <set>
#include <iomanip>

#include "TableDrawer.h"
#include "TraceDrawer.h"

const std::string EPSILON = "''";
const std::string ENDOF = "$";

enum Mode { NORMAL, CHOCOPY };

struct Production {
    int id;
    std::string noTerminal;
    std::vector<std::string> derivacion;
};

struct LR1Item {
    int productionId;
    int dot;
    std::string lookahead;

    bool operator<(const LR1Item& o) const {
        if (productionId != o.productionId) return productionId < o.productionId;
        if (dot != o.dot) return dot < o.dot;
        return lookahead < o.lookahead;
    }
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

struct FollowingTable {
    std::stack<int> states;
    std::stack<std::string> stac;
    std::queue<std::string> input;
    std::string action;
};

struct PanicConfig {
    std::vector<std::string> noTerminales;
    std::unordered_set<std::string> syncTokens;
    std::vector<int> gotosNum;
    bool initialized = false;
};

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
    std::unordered_map<std::string, std::unordered_set<std::string>> followSet;

    std::string startSymbol;
    std::string augmentedStart = "S'";

    std::vector<Transition> transitions;
    std::vector<StateRow> table;

    PanicConfig panicConfig;
public:
    std::vector<Conflicto> conflictos;
    Mode mode = NORMAL;
public:
    Parser(std::filesystem::path inBaseDir) : baseDir(inBaseDir) {};

    void CleanGrammar() {
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

        followSet.clear();
        table.clear();
        panicConfig = PanicConfig{};

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

    void RegisterConflicts(int estado, const std::string& simbolo, const std::string& existente, const std::string& nueva) {
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

        conflictos.push_back({ estado, simbolo, existente, nueva, tipo });
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
            while (iter < size && (buff[iter] == '\n' || buff[iter] == '\r' || buff[iter] == ' ' || buff[iter] == '\t')) {
                iter++;
            }
            if (iter >= size) break;

            std::string tmpBuffer = {};
            Production tmpProd = {};

            while (iter < size && buff[iter] != ' ') {
                tmpBuffer += buff[iter];
                iter++;
            }

            tmpProd.noTerminal = tmpBuffer;
            tmpBuffer = {};

            if (!tmpProd.noTerminal.empty()) {
                if (std::find(ordenSimbolos.begin(), ordenSimbolos.end(), tmpProd.noTerminal) == ordenSimbolos.end()) {
                    ordenSimbolos.push_back(tmpProd.noTerminal);
                }

                noTerminales.insert(tmpProd.noTerminal);
                if (startSymbol.empty()) {
                    startSymbol = tmpProd.noTerminal;
                }
            }

            iter++;

            while (iter < size && buff[iter] == ' ') {
                iter++;
            }

            if (iter + 1 < size && buff[iter] == '-' && buff[iter + 1] == '>') {
                iter += 2;
            }
            else {
                throw std::runtime_error("Error en la gramatica: se esperaba '->' despues del no terminal");
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
                            if (std::find(ordenSimbolos.begin(), ordenSimbolos.end(), tmpBuffer) == ordenSimbolos.end()) {

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
                    if (std::find(ordenSimbolos.begin(), ordenSimbolos.end(), tmpBuffer) == ordenSimbolos.end()) {
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

        augmented.id = static_cast<int>(productions.size());
        augmented.noTerminal = augmentedStart;
        augmented.derivacion.push_back(startSymbol);

        productions.push_back(augmented);

        grammar[augmentedStart].insert(grammar[augmentedStart].begin(), augmented);
        noTerminales.insert(augmentedStart);
    }

    void LoadFile(const std::filesystem::path& filePath) {
        CleanGrammar();
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

    bool AddFirst(const std::string& A, const std::unordered_set<std::string>& nuevos) {
        bool changed = false;
        for (auto it = nuevos.begin(); it != nuevos.end();it++) {

            if (firstSet[A].find(*it) == firstSet[A].end()) {

                firstSet[A].insert(*it);
                changed = true;
            }
        }

        return changed;
    }

    void InsertWithoutEps(const std::string& A, std::unordered_set<std::string>& nuevos) {
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
                    int size = static_cast<int>(prod.derivacion.size());
                    std::unordered_set<std::string> rhs;
                    bool todosEpsilon = true;
                    for (int i = 0; i < size; i++) {
                        std::string X = prod.derivacion[i];

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
            InsertWithoutEps(simbolo, resultado);

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

    bool SameItem(const LR1Item& a, const LR1Item& b) {
        return a.productionId == b.productionId && a.dot == b.dot && a.lookahead == b.lookahead;
    }

    bool AddItem(ItemSet& items, const LR1Item& nuevo) {
        for (auto& actual : items) {
            if (SameItem(actual, nuevo)) {
                return false;
            }
        }

        items.push_back(nuevo);

        return true;
    }

    ItemSet closure(const ItemSet& kernel) {
        ItemSet result = kernel;
        std::set<LR1Item> vistos(kernel.begin(), kernel.end());

        for (int i = 0; i < result.size(); i++) {
            LR1Item p = result[i];
            Production& prod = productions[p.productionId];

            if (p.dot >= static_cast<int>(prod.derivacion.size())) {
                continue;
            }

            std::string& symbol = prod.derivacion[p.dot];
            if (noTerminales.find(symbol) == noTerminales.end()) {
                continue;
            }

            std::vector<std::string> cadena(prod.derivacion.begin() + p.dot + 1, prod.derivacion.end());
            cadena.push_back(p.lookahead);

            std::unordered_set<std::string> primero = firstCadena(cadena);

            for (const auto& pp : grammar[symbol]) {
                for (const auto& prim : primero) {
                    LR1Item nuevo{ pp.id, 0, prim };

                    if (vistos.insert(nuevo).second) {
                        result.push_back(nuevo);
                    }
                }
            }
        }

        std::sort(result.begin(), result.end());
        return result;
    }

    ItemSet gotto(const ItemSet& item, const std::string& X) {
        ItemSet kernel;

        for (const auto& itm : item) {
            const auto& der = productions[itm.productionId].derivacion;

            if (itm.dot < static_cast<int>(der.size()) && der[itm.dot] == X) {
                kernel.push_back({ itm.productionId, itm.dot + 1, itm.lookahead });
            }
        }

        if (kernel.empty()) {
            return kernel;
        }

        return closure(kernel);
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

    int getProductionCount() const {
        return static_cast<int>(productions.size());
    }

    std::vector<ItemSet> coleccionCanonica() {
        transitions.clear();
        if ((int)firstSet.size() <= 0) {
            first();
        }

        std::vector<ItemSet> CCs;
        std::map<ItemSet, int> indice;

        LR1Item itemInicial = { static_cast<int>(productions.size()) - 1, 0, "$" };
        ItemSet estadoInicial = closure(ItemSet{ itemInicial });

        CCs.push_back(estadoInicial);
        indice[estadoInicial] = 0;

        for (int i = 0; i < static_cast<int>(CCs.size()); i++) {
            std::unordered_set<std::string> despuesDelPunto;
            for (const auto& it : CCs[i]) {
                const auto& der = productions[it.productionId].derivacion;
                if (it.dot < static_cast<int>(der.size())) {
                    despuesDelPunto.insert(der[it.dot]);
                }
            }

            for (const auto& X : ordenSimbolos) {
                if (despuesDelPunto.find(X) == despuesDelPunto.end()) {
                    continue;
                }

                ItemSet new_CC = gotto(CCs[i], X);

                if (new_CC.empty()) {
                    continue;
                }

                int destino;
                auto it = indice.find(new_CC);

                if (it == indice.end()) {
                    destino = static_cast<int>(CCs.size());
                    indice[new_CC] = destino;
                    CCs.push_back(std::move(new_CC));
                }
                else {
                    destino = it->second;
                }

                transitions.push_back({ i, X, destino });
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

    void agregarAccion(std::map<std::string, std::string>& actionMap, int estado, const std::string& simbolo, const std::string& accion) {
        auto it = actionMap.find(simbolo);

        if (it == actionMap.end()) {
            actionMap[simbolo] = accion;
        }
        else if (it->second != accion) {
            actionMap[simbolo] += "/" + accion;
            RegisterConflicts(estado, simbolo, it->second, accion);
        }
    }

    void llenarTablas(const std::string& nombreArchivo = "tabla_parseo.html") {
        std::cout << " ======== LLENANDO TABLAS ========= " << std::endl;
        table.clear();
        conflictos.clear();

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
                    if (prod.noTerminal == augmentedStart && cc.lookahead == ENDOF) {
                        agregarAccion(actionMap, i, ENDOF, "acc");
                    }
                    else {
                        agregarAccion(actionMap, i, cc.lookahead, "r" + std::to_string(cc.productionId + 1));
                    }
                }
            }

            table.push_back(StateRow{ i,actionMap,gotoMap });
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

            if (prueba.productions.empty() || prueba.startSymbol.empty()) {
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

    void follow() {
        if (firstSet.empty()) return;

        for (auto NT : noTerminales) {
            followSet[NT];
        }
        followSet[augmentedStart].insert(ENDOF);

        bool cambio = true;
        while (cambio) {
            cambio = false;

            for (Production prod : productions) {
                std::string noTerminal = prod.noTerminal;
                std::unordered_set<std::string> trailer = followSet[noTerminal];

                std::vector<std::string>& deri = prod.derivacion;
                for (int i = deri.size() - 1;i >= 0;i--) {
                    std::string simbolo = deri[i];

                    if (noTerminales.find(simbolo) != noTerminales.end()) {
                        int tamanoPrevio = followSet[simbolo].size();
                        followSet[simbolo].insert(trailer.begin(), trailer.end());

                        if (followSet[simbolo].size() > tamanoPrevio) {
                            cambio = true;
                        }

                        if (HasEpsilon(simbolo)) {
                            for (auto elem : firstSet[simbolo]) {
                                if (elem != EPSILON) {
                                    trailer.insert(elem);
                                }
                            }
                        }
                        else {
                            trailer = firstSet[simbolo];
                        }
                    }
                    else {
                        trailer = { simbolo };
                    }
                }
            }
        }
    }

    void PrintFollowSet() {
        std::cout << std::endl << "FOLLOW sets : " << std::endl;

        for (auto& entrada : followSet) {
            std::cout << "FOLLOW(" << entrada.first << ") = { ";

            for (auto& simbolo : entrada.second) {
                std::cout << simbolo << " ";
            }
            std::cout << "}" << std::endl;
        }
    }

    void initializePanicMode(const std::vector<std::string>& seleccionados = {})
    {
        panicConfig = PanicConfig{};

        if (firstSet.empty()) {
            first();
        }

        if (followSet.empty()) {
            follow();
        }

        for (const std::string& nt : seleccionados) {
            if (noTerminales.find(nt) == noTerminales.end() || nt == augmentedStart) {
                throw std::runtime_error("Modo panico: '" + nt + "' no es un no terminal valido");
            }

            if (std::find(panicConfig.noTerminales.begin(),panicConfig.noTerminales.end(),nt) == panicConfig.noTerminales.end()) {
                panicConfig.noTerminales.push_back(nt);
            }

            auto it = followSet.find(nt);
            if (it != followSet.end()) {
                panicConfig.syncTokens.insert(it->second.begin(),it->second.end());
            }
        }

        for (int i = 0; i < static_cast<int>(table.size()); i++) {
            for (const auto& entrada : table[i].gotoMap) {
                if (std::find(panicConfig.noTerminales.begin(),panicConfig.noTerminales.end(),entrada.first) != panicConfig.noTerminales.end()) {
                    panicConfig.gotosNum.push_back(i);
                    break;
                }
            }
        }

        panicConfig.initialized = true;

        std::cout << std::endl << "=== Configuracion del modo panico ===" << std::endl;
        std::cout << "No terminales seleccionados: ";
        for (std::string& nt : panicConfig.noTerminales) {
            std::cout << nt << " ";
        }
        std::cout << std::endl;

        std::cout << "Tokens de sincronizacion: ";
        for (const std::string& token : panicConfig.syncTokens) {
            std::cout << token << " ";
        }
        std::cout << std::endl;

        std::cout << "Estados con GOTO: ";
        for (int estado : panicConfig.gotosNum) {
            std::cout << estado << " ";
        }
        std::cout << std::endl;
        std::cout << "====================================" << std::endl;;
    }

    void panicMode(std::vector<FollowingTable>& seguimiento, FollowingTable& actualEntry)
    {
        if (!panicConfig.initialized) {
            initializePanicMode();
        }

        auto& syncTokens = panicConfig.syncTokens;
        auto& gotos = panicConfig.gotosNum;

        std::string top;
        while (!actualEntry.input.empty()) {
            top = actualEntry.input.front();

            if (syncTokens.find(top) != syncTokens.end()) {
                break;
            }
            actualEntry.input.pop();
        }

        if (actualEntry.input.empty()) {
            actualEntry.action = "Error: no se encontro un token de sincronizacion";
            seguimiento.push_back(actualEntry);
            return;
        }
        actualEntry.action = "Se consumio el input hasta encontrar el token de sincronizacion: " + top;
        seguimiento.push_back(actualEntry);

        // Desapilar hasta encontrar un estado con GOTO valido
        while (!actualEntry.states.empty())
        {
            int estado = actualEntry.states.top();
            auto itEstado = std::find(gotos.begin(), gotos.end(), estado);

            if (itEstado != gotos.end())
            {
                auto& gotoMap = table[estado].gotoMap;

                for (auto& entrada : gotoMap)
                {
                    if (std::find(panicConfig.noTerminales.begin(), panicConfig.noTerminales.end(), entrada.first) != panicConfig.noTerminales.end()) {
                        actualEntry.stac.push(entrada.first);
                        actualEntry.states.push(std::stoi(entrada.second));
                        actualEntry.action = "Se encontro un GOTO desde el estado " + std::to_string(estado) + " hacia " + entrada.first + ", estado destino " + entrada.second;

                        seguimiento.push_back(actualEntry);
                        return;
                    }
                }
            }
            actualEntry.states.pop();
            actualEntry.stac.pop();
        }

        actualEntry.action = "Error: no se encontro un estado con GOTO valido";
        seguimiento.push_back(actualEntry);
    }

    void printStack(std::stack<int> pila) {
        while (!pila.empty()) {
            std::cout << pila.top() << " ";
            pila.pop();
        }
    }

    void printStack(std::stack<std::string> pila) {
        while (!pila.empty()) {
            std::cout << pila.top() << " ";
            pila.pop();
        }
    }

    void printFollowing(const std::vector<FollowingTable>& followTable) {
        std::cout << std::endl << std::left << std::setw(25) << "Estados" << std::setw(20) << "Pila" << std::setw(25) << "Entrada" << "Accion" << std::endl;

        for (const FollowingTable& row : followTable) {
            std::stack<int> states = row.states;
            std::stack<std::string> stac = row.stac;
            std::queue<std::string> input = row.input;

            std::string estados, pila, entrada;

            while (!states.empty()) {
                estados = std::to_string(states.top()) + " " + estados;
                states.pop();
            }

            while (!stac.empty()) {
                pila = stac.top() + " " + pila;
                stac.pop();
            }

            while (!input.empty()) {
                entrada += input.front() + " ";
                input.pop();
            }
            std::cout << std::left << std::setw(25) << estados << std::setw(20) << pila << std::setw(25) << entrada << row.action << std::endl;
        }
    }

    std::vector<TraceRow> toTraceRows(const std::vector<FollowingTable>& followTable)
    {
        std::vector<TraceRow> rows;

        for (const FollowingTable& row : followTable) {
            std::stack<int> states = row.states;
            std::stack<std::string> stac = row.stac;

            std::string estados, pila;

            while (!states.empty()) {
                estados = std::to_string(states.top()) + " " + estados;
                states.pop();
            }
            while (!stac.empty()) {
                pila = stac.top() + " " + pila;
                stac.pop();
            }

            std::string entrada = row.input.empty() ? "-" : row.input.front();

            rows.push_back({ estados, pila, entrada, row.action });
        }

        return rows;
    }

    bool checkAction(int CCs, std::string input)
    {
        return table[CCs].actionMap.find(input) != table[CCs].actionMap.end();
    }

    std::string nextWord(std::queue<std::string>& wordQueue)
    {
        if (wordQueue.empty())
            return "";

        std::string word = wordQueue.front();
        wordQueue.pop();
        return word;
    }

    void following(const std::string& input, const std::string &file_path = "trace_table.html")
    {
        if (input.empty())
            throw std::runtime_error("Error de entrada: la cadena de entrada esta vacia");

        if (table.empty())
            throw std::runtime_error("Error del parser: la tabla LR(1) no ha sido generada");

        std::queue<std::string> que;
        std::stringstream ss(input);
        std::string word;

        while (ss >> word)
            que.push(word);
        
        que.push(ENDOF);

        word = nextWord(que);

        std::vector<FollowingTable> followTable;

        FollowingTable Current;
        Current.states.push(0);
        Current.stac.push(ENDOF);
        Current.input = que;

        while (true)
        {
            if (Current.states.empty())
                throw std::runtime_error("Error interno del parser: la pila de estados esta vacia");

            int state = Current.states.top();

            if (!checkAction(state, word))
            {
                Current.action = "Error sintactico: token inesperado (MODO PANICO) ";

                std::queue<std::string> nuevaEntrada;
                nuevaEntrada.push(word);

                while (!Current.input.empty())
                {
                    nuevaEntrada.push(Current.input.front());
                    Current.input.pop();
                }

                Current.input = nuevaEntrada;
                followTable.push_back(Current);

                panicMode(followTable, Current);
                if (Current.states.empty() || Current.input.empty())
                    break;

                word = nextWord(Current.input);
                continue;
            }

            Current.action = table[state].actionMap[word];

            FollowingTable fila = Current;
            std::queue<std::string> entradaActual;
            entradaActual.push(word);

            std::queue<std::string> restante = Current.input;
            while (!restante.empty())
            {
                entradaActual.push(restante.front());
                restante.pop();
            }

            fila.input = entradaActual;
            followTable.push_back(fila);

            if (Current.action[0] == 'r')
            {
                int toRemove = std::stoi(Current.action.substr(1)) - 1;
                Production produccion = productions[toRemove];

                std::string toPut = produccion.noTerminal;
                int sizeRemove = produccion.derivacion.size();

                for (int i = 0; i < sizeRemove; i++)
                {
                    Current.states.pop();
                    Current.stac.pop();
                }

                if (Current.states.empty())
                    throw std::runtime_error("Caso raro: se fueron todos los estados");


                int estadoActual = Current.states.top();
                Current.states.push(
                    std::stoi(table[estadoActual].gotoMap[toPut])
                );
                Current.stac.push(toPut);
            }
            else if (Current.action[0] == 's')
            {
                int toRemove = std::stoi(Current.action.substr(1));

                Current.stac.push(word);
                Current.states.push(toRemove);

                word = nextWord(Current.input);
            }
            else if (Current.action == "acc")
            {
                std::cout << "Aceptado!!!!" << std::endl;
                break;
            }
            else
                throw std::runtime_error("Fallo :(");
        }

        printFollowing(followTable);

        TraceDrawer drawer{ baseDir };
        drawer.saveToFile(file_path, toTraceRows(followTable));
    }

    void get_trace_table(const Scanner& inScanner, const std::string& file_path = "trace_table.html")
    {
        if (table.empty())
            throw std::runtime_error("Parser ERROR: LR(1) table not generated");

        
        std::string word;


        word = nextWord(que);

        std::vector<FollowingTable> followTable;

        FollowingTable Current;
        Current.states.push(0);
        Current.stac.push(ENDOF);
        Current.input = que;

        while (true)
        {
            if (Current.states.empty())
                throw std::runtime_error("Error interno del parser: la pila de estados esta vacia");

            int state = Current.states.top();

            if (!checkAction(state, word))
            {
                Current.action = "Error sintactico: token inesperado (MODO PANICO) ";

                std::queue<std::string> nuevaEntrada;
                nuevaEntrada.push(word);

                while (!Current.input.empty())
                {
                    nuevaEntrada.push(Current.input.front());
                    Current.input.pop();
                }

                Current.input = nuevaEntrada;
                followTable.push_back(Current);

                panicMode(followTable, Current);
                if (Current.states.empty() || Current.input.empty())
                    break;

                word = nextWord(Current.input);
                continue;
            }

            Current.action = table[state].actionMap[word];

            FollowingTable fila = Current;
            std::queue<std::string> entradaActual;
            entradaActual.push(word);

            std::queue<std::string> restante = Current.input;
            while (!restante.empty())
            {
                entradaActual.push(restante.front());
                restante.pop();
            }

            fila.input = entradaActual;
            followTable.push_back(fila);

            if (Current.action[0] == 'r')
            {
                int toRemove = std::stoi(Current.action.substr(1)) - 1;
                Production produccion = productions[toRemove];

                std::string toPut = produccion.noTerminal;
                int sizeRemove = produccion.derivacion.size();

                for (int i = 0; i < sizeRemove; i++)
                {
                    Current.states.pop();
                    Current.stac.pop();
                }

                if (Current.states.empty())
                    throw std::runtime_error("Caso raro: se fueron todos los estados");


                int estadoActual = Current.states.top();
                Current.states.push(
                    std::stoi(table[estadoActual].gotoMap[toPut])
                );
                Current.stac.push(toPut);
            }
            else if (Current.action[0] == 's')
            {
                int toRemove = std::stoi(Current.action.substr(1));

                Current.stac.push(word);
                Current.states.push(toRemove);

                word = nextWord(Current.input);
            }
            else if (Current.action == "acc")
            {
                std::cout << "Aceptado!!!!" << std::endl;
                break;
            }
            else
                throw std::runtime_error("Fallo :(");
        }

        printFollowing(followTable);

        TraceDrawer drawer{ baseDir };
        drawer.saveToFile(file_path, toTraceRows(followTable));
    }



};