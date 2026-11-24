#ifndef SEMANTICANALYZER_H
#define SEMANTICANALYZER_H

#include <iostream>
#include <string>
#include <vector>

#include "Parser.h"
#include "ErrorManager.h"
#include "SemanticValue.h"
#include "SymbolTable.h"
#include "TypeRules.h"

struct UsoPendiente
{
	std::string name;
	Scope* scope;
	Position position;
};

class SemanticAnalyzer : public ReduceListener
{
public:
	bool debug = false;

	void setErrorManager(ErrorManager* manager)
	{
		errorManager = manager;
	}

	void reset()
	{
		symbols.reset();
		pendientes.clear();
		claseActual.clear();
		raiz = nullptr;
	}

	const ASTPtr& getAst() const
	{
		return raiz;
	}

	void printSymbols() const
	{
		symbols.print();
	}

	const SymbolTable& getSymbols() const
	{
		return symbols;
	}

	// panic mode heades scopes fix
	void onDiscard(const SemValue& valor) override
	{
		if (valor.symbol == "func_head")
		{
			symbols.closeScope();
		}
		else if (valor.symbol == "class_head")
		{
			symbols.closeScope();
			claseActual.clear();
		}
	}

	void onReduce(const Production& prod, const std::vector<SemValue>& hijos, SemValue& resultado) override
	{
		if (debug)
			printReduce(prod, hijos);

		const std::string& nt = prod.noTerminal;

		if (hijos.size() == 1)
		{
			resultado.type = hijos[0].type;
			resultado.node = hijos[0].node;
		}

		if (nt == "type")
			reduceType(prod, hijos, resultado);
		else if (nt == "typed_var")
			resultado.type = hijos[2].type;
		else if (nt == "params")
			reduceParams(prod, hijos, resultado);
		else if (nt == "params_opt")
			reduceParamsOpt(prod, hijos, resultado);
		else if (nt == "ret_opt")
			reduceRetOpt(prod, hijos, resultado);
		else if (nt == "var_def")
			reduceVarDef(hijos, resultado);
		else if (nt == "func_head")
			reduceFuncHead(hijos, resultado);
		else if (nt == "func_def")
			reduceFuncDef(hijos, resultado);
		else if (nt == "func_body")
			reduceFuncBody(hijos, resultado);
		else if (nt == "class_head")
			reduceClassHead(hijos, resultado);
		else if (nt == "class_def")
			reduceClassDef(hijos, resultado);
		else if (nt == "class_body")
			reduceClassBody(prod, hijos, resultado);
		else if (nt == "global_decl")
			reduceRefDecl(hijos, SymbolKind::GLOBAL_REF, resultado);
		else if (nt == "nonlocal_decl")
			reduceRefDecl(hijos, SymbolKind::NONLOCAL_REF, resultado);
		else if (esLista(nt))
			reduceLista(hijos, resultado);
		else if (nt == "literal")
			reduceLiteral(prod, resultado);
		else if (nt == "primary")
			reducePrimary(prod, hijos, resultado);
		else if (nt == "member_expr")
			reduceMember(hijos, resultado);
		else if (nt == "index_expr")
			reduceIndex(hijos, resultado);
		else if (nt == "expr_list")
			reduceExprList(prod, hijos, resultado);
		else if (nt == "expr_list_opt" && !prod.derivacion.empty())
			resultado.items = hijos[0].items;
		else if (esOperadorBinario(nt) && hijos.size() == 3)
			reduceBinario(hijos, resultado);
		else if ((nt == "unary" || nt == "not_expr") && hijos.size() == 2)
			reduceUnario(hijos, resultado);
		else if (nt == "target" && prod.derivacion[0] == "ID")
		{
			resultado.type = tipoDeVariable(asignarNombre(hijos[0].token));
			resultado.node = makeNode("Id " + hijos[0].token.lexeme, hijos[0].token.position);
		}
		else if (nt == "assign_list")
			reduceAssignList(prod, hijos, resultado);
		else if (nt == "expr" && hijos.size() == 5)
			reduceCondicional(hijos, resultado);
		else if (nt == "simple_stmt")
			reduceSimpleStmt(prod, hijos, resultado);
		else if (nt == "stmt")
			reduceStmt(prod, hijos, resultado);
		else if (nt == "block")
		{
			resultado.node = makeNode("Block", hijos[1].token.position);
			agregarItems(resultado.node, hijos[2]);
		}
		else if (nt == "elif_list" && hijos.size() == 5)
			reduceElif(hijos, resultado);
		else if (nt == "else_opt" && hijos.size() == 3)
		{
			resultado.node = makeNode("Else", hijos[0].token.position);
			resultado.node->addChild(hijos[2].node);
		}
		else if (nt == "program")
			reduceProgram(hijos, resultado);

		if (resultado.node && !resultado.type.empty())
			resultado.node->type = resultado.type;
	}

private:
	ErrorManager* errorManager = nullptr;
	SymbolTable symbols;
	std::vector<UsoPendiente> pendientes;
	std::string claseActual;
	ASTPtr raiz;

	// type -> ID | IDSTRING | [ type ]
	void reduceType(const Production& prod, const std::vector<SemValue>& hijos, SemValue& resultado)
	{
		const std::string& primero = prod.derivacion[0];

		if (primero == "ID")
			resultado.type = hijos[0].token.lexeme;
		else if (primero == "IDSTRING")
			resultado.type = quitarComillas(hijos[0].token.lexeme);
		else if (primero == "[")
			resultado.type = "[" + hijos[1].type + "]";
	}

	// params -> typed_var | params , typed_var
	void reduceParams(const Production& prod, const std::vector<SemValue>& hijos, SemValue& resultado)
	{
		if (prod.derivacion.size() == 1)
		{
			agregarParametro(resultado, hijos[0]);
		}
		else
		{
			copiarParametros(resultado, hijos[0]);
			agregarParametro(resultado, hijos[2]);
		}
	}

	// params_opt -> params | ''
	void reduceParamsOpt(const Production& prod, const std::vector<SemValue>& hijos, SemValue& resultado)
	{
		if (!prod.derivacion.empty())
			copiarParametros(resultado, hijos[0]);
	}

	// ret_opt -> -> type | ''
	void reduceRetOpt(const Production& prod, const std::vector<SemValue>& hijos, SemValue& resultado)
	{
		if (prod.derivacion.empty())
			resultado.type = "<None>";
		else
			resultado.type = hijos[1].type;
	}

	bool enClase()
	{
		return symbols.currentScope()->kind == ScopeKind::CLASS;
	}

	// var_def -> typed_var = literal NEWLINE
	void reduceVarDef(const std::vector<SemValue>& hijos, SemValue& resultado)
	{
		const SemValue& var = hijos[0];

		SymbolEntry entrada;
		entrada.name = var.token.lexeme;
		entrada.kind = enClase() ? SymbolKind::ATTRIBUTE : SymbolKind::VARIABLE;
		entrada.type = var.type;
		entrada.position = var.token.position;

		declarar(entrada);

		const SemValue& valor = hijos[2];
		if (!esAsignable(valor.type, var.type))
			reportarError(valor.token.position, "no se puede inicializar '" + entrada.name + "' de tipo " +
				var.type + " con un valor de tipo " + valor.type);

		resultado.node = makeNode("VarDef " + entrada.name + ": " + var.type, entrada.position);
		resultado.node->addChild(valor.node);
	}

	// func_head -> def ID ( params_opt ) ret_opt :
	void reduceFuncHead(const std::vector<SemValue>& hijos, SemValue& resultado)
	{
		const SemValue& nombre = hijos[1];
		const SemValue& params = hijos[3];

		SymbolEntry funcion;
		funcion.name = nombre.token.lexeme;
		bool esMetodo = enClase();
		funcion.kind = esMetodo ? SymbolKind::METHOD : SymbolKind::FUNCTION;
		funcion.type = hijos[5].type;
		funcion.position = nombre.token.position;
		funcion.paramNames = params.paramNames;
		funcion.paramTypes = params.paramTypes;

		if (esMetodo)
			verificarPrimerParametro(funcion);

		SymbolEntry* entrada = declarar(funcion);

		std::string prefijo = esMetodo ? "metodo " : "funcion ";
		Scope* cuerpo = symbols.openScope(prefijo + funcion.name, ScopeKind::FUNCTION);

		if (entrada)
			entrada->bodyScopeId = cuerpo->id;

		for (size_t i = 0; i < params.paramNames.size(); i++)
		{
			SymbolEntry parametro;
			parametro.name = params.paramNames[i];
			parametro.kind = SymbolKind::PARAMETER;
			parametro.type = params.paramTypes[i];
			parametro.position = params.paramPositions[i];

			declarar(parametro);
		}

		std::string firma = "FuncDef " + funcion.name + "(";
		for (size_t i = 0; i < params.paramNames.size(); i++)
		{
			if (i > 0)
				firma += ", ";
			firma += params.paramNames[i] + ": " + params.paramTypes[i];
		}
		firma += ") -> " + funcion.type;

		resultado.node = makeNode(firma, funcion.position);
	}

	void reduceFuncDef(const std::vector<SemValue>& hijos, SemValue& resultado)
	{
		symbols.closeScope();

		resultado.node = hijos[0].node;
		if (resultado.node)
			agregarItems(resultado.node, hijos[3]);
	}

	void reduceFuncBody(const std::vector<SemValue>& hijos, SemValue& resultado)
	{
		resultado.node = nullptr;
		resultado.items = hijos[0].items;
		resultado.items.insert(resultado.items.end(), hijos[1].items.begin(), hijos[1].items.end());
	}

	// class_head -> class ID ( ID ) :
	void reduceClassHead(const std::vector<SemValue>& hijos, SemValue& resultado)
	{
		const SemValue& nombre = hijos[1];

		SymbolEntry clase;
		clase.name = nombre.token.lexeme;
		clase.kind = SymbolKind::CLASS;
		clase.type = clase.name;
		clase.position = nombre.token.position;
		clase.superClass = hijos[3].token.lexeme;

		if (!verificarSuperclase(hijos[3].token))
			clase.superClass = "object";

		SymbolEntry* entrada = declarar(clase);

		Scope* cuerpo = symbols.openScope("clase " + clase.name, ScopeKind::CLASS);
		if (entrada)
			entrada->bodyScopeId = cuerpo->id;
		
		claseActual = clase.name;

		resultado.node = makeNode("ClassDef " + clase.name + "(" + clase.superClass + ")", clase.position);
	}

	void reduceClassDef(const std::vector<SemValue>& hijos, SemValue& resultado)
	{
		symbols.closeScope();
		claseActual.clear();

		resultado.node = hijos[0].node;
		if (resultado.node)
		{
			resultado.node->addChild(hijos[3].node);
			agregarItems(resultado.node, hijos[3]);
		}
	}

	void reduceClassBody(const Production& prod, const std::vector<SemValue>& hijos, SemValue& resultado)
	{
		if (prod.derivacion[0] == "pass")
		{
			resultado.node = makeNode("Pass", hijos[0].token.position);
		}
		else
		{
			resultado.node = nullptr;
			resultado.items = hijos[0].items;
		}
	}

	bool verificarSuperclase(const Token& super)
	{
		const std::string& nombre = super.lexeme;
		SymbolEntry* entrada = symbols.globalScope()->table.find(nombre);

		if (entrada == nullptr)
			reportarError(super.position, "la superclase '" + nombre + "' no esta definida");
		else if (entrada->kind != SymbolKind::CLASS)
			reportarError(super.position, "'" + nombre + "' no es una clase, no se puede heredar de ella");
		else if (nombre == "int" || nombre == "bool" || nombre == "str")
			reportarError(super.position, "no se puede heredar de la clase especial '" + nombre + "'");
		else
			return true;

		return false;
	}

	void verificarPrimerParametro(const SymbolEntry& metodo)
	{
		if (metodo.paramTypes.empty() || metodo.paramTypes[0] != claseActual)
			reportarError(metodo.position, "el primer parametro del metodo '" + metodo.name +
				"' debe ser de tipo " + claseActual);
	}

	void verificarReturn(const Token& ret)
	{
		if (symbols.currentScope()->kind == ScopeKind::GLOBAL)
			reportarError(ret.position, "'return' fuera de una funcion");
	}

	// global_decl   -> global ID NEWLINE
	// nonlocal_decl -> nonlocal ID NEWLINE
	void reduceRefDecl(const std::vector<SemValue>& hijos, SymbolKind kind, SemValue& resultado)
	{
		const SemValue& nombre = hijos[1];

		SymbolEntry ref;
		ref.name = nombre.token.lexeme;
		ref.kind = kind;
		ref.position = nombre.token.position;

		SymbolEntry* destino = nullptr;
		if (kind == SymbolKind::GLOBAL_REF)
			destino = symbols.globalScope()->table.find(ref.name);
		else
			destino = symbols.lookup(ref.name, symbols.currentScope()->parent);

		if (destino && (destino->kind == SymbolKind::VARIABLE || destino->kind == SymbolKind::PARAMETER))
			ref.type = destino->type;

		declarar(ref);

		std::string palabra = (kind == SymbolKind::GLOBAL_REF) ? "Global " : "Nonlocal ";
		resultado.node = makeNode(palabra + ref.name, ref.position);
	}

	static bool esLista(const std::string& nt)
	{
		return nt == "decl_list" || nt == "func_decl_list" || nt == "stmt_plus" || nt == "class_item_list";
	}

	void reduceLista(const std::vector<SemValue>& hijos, SemValue& resultado)
	{
		resultado.node = nullptr;

		if (hijos.size() == 1)
		{
			resultado.items.push_back(hijos[0]);
		}
		else if (hijos.size() == 2)
		{
			resultado.items = hijos[0].items;
			resultado.items.push_back(hijos[1]);
		}
	}

	void reduceSimpleStmt(const Production& prod, const std::vector<SemValue>& hijos, SemValue& resultado)
	{
		const std::string& primero = prod.derivacion[0];

		if (primero == "pass")
		{
			resultado.node = makeNode("Pass", hijos[0].token.position);
		}
		else if (primero == "return")
		{
			verificarReturn(hijos[0].token);
			resultado.node = makeNode("Return", hijos[0].token.position);
			resultado.node->addChild(hijos[1].node);
		}
		else if (primero == "assign_list")
		{
			verificarAsignacion(hijos[0], hijos[1]);
			resultado.node = makeNode("Assign", hijos[1].token.position);
			agregarItems(resultado.node, hijos[0]);
			resultado.node->addChild(hijos[1].node);
		}
	}

	void reduceStmt(const Production& prod, const std::vector<SemValue>& hijos, SemValue& resultado)
	{
		const std::string& primero = prod.derivacion[0];
		const Position& pos = hijos[0].token.position;

		if (primero == "simple_stmt")
		{
			resultado.node = hijos[0].node;
		}
		else if (primero == "if")
		{
			verificarCondicion(hijos[1], "if");
			resultado.node = makeNode("If", pos);
			resultado.node->addChild(hijos[1].node);
			resultado.node->addChild(hijos[3].node);
			agregarItems(resultado.node, hijos[4]);
			resultado.node->addChild(hijos[5].node);
		}
		else if (primero == "while")
		{
			verificarCondicion(hijos[1], "while");
			resultado.node = makeNode("While", pos);
			resultado.node->addChild(hijos[1].node);
			resultado.node->addChild(hijos[3].node);
		}
		else if (primero == "for")
		{
			asignarNombre(hijos[1].token);
			resultado.node = makeNode("For", pos);
			resultado.node->addChild(makeNode("Id " + hijos[1].token.lexeme, hijos[1].token.position));
			resultado.node->addChild(hijos[3].node);
			resultado.node->addChild(hijos[5].node);
		}
	}

	void reduceElif(const std::vector<SemValue>& hijos, SemValue& resultado)
	{
		verificarCondicion(hijos[2], "elif");

		SemValue elif = hijos[1];
		elif.node = makeNode("Elif", hijos[1].token.position);
		elif.node->addChild(hijos[2].node);
		elif.node->addChild(hijos[4].node);

		resultado.items = hijos[0].items;
		resultado.items.push_back(elif);
	}

	void reduceProgram(const std::vector<SemValue>& hijos, SemValue& resultado)
	{
		resolverPendientes();
		verificarTabla();

		raiz = makeNode("Program", Position(1, 1));
		for (const SemValue& h : hijos)
			agregarItems(raiz, h);

		resultado.node = raiz;
	}

	void reduceLiteral(const Production& prod, SemValue& resultado)
	{
		const std::string& valor = prod.derivacion[0];
		std::string etiqueta;

		if (valor == "None")
		{
			resultado.type = "<None>";
			etiqueta = "None";
		}
		else if (valor == "True" || valor == "False")
		{
			resultado.type = "bool";
			etiqueta = "Bool ";
		}
		else if (valor == "INTEGER")
		{
			resultado.type = "int";
			etiqueta = "Num ";
		}
		else
		{
			resultado.type = "str";
			etiqueta = "Str ";
		}

		if (valor != "None")
			etiqueta += resultado.token.lexeme;

		resultado.node = makeNode(etiqueta, resultado.token.position);
	}

	void reducePrimary(const Production& prod, const std::vector<SemValue>& hijos, SemValue& resultado)
	{
		const std::vector<std::string>& d = prod.derivacion;
		const Token& primero = hijos[0].token;

		if (d[0] == "ID" && d.size() == 1)
		{
			resultado.type = tipoDeVariable(usarNombre(primero));
			resultado.node = makeNode("Id " + primero.lexeme, primero.position);
		}
		else if (d[0] == "ID")
		{
			resultado.type = tipoDeLlamada(usarNombre(primero));
			resultado.node = makeNode("Call " + primero.lexeme, primero.position);
			agregarItems(resultado.node, hijos[2]);
		}
		else if (d[0] == "member_expr" && d.size() == 4)
		{
			resultado.node = makeNode("MethodCall", hijos[1].token.position);
			resultado.node->addChild(hijos[0].node);
			agregarItems(resultado.node, hijos[2]);
		}
		else if (d[0] == "(")
		{
			resultado.type = hijos[1].type;
			resultado.node = hijos[1].node;
		}
		else if (d[0] == "[")
		{
			resultado.node = makeNode("ListExpr", primero.position);
			agregarItems(resultado.node, hijos[1]);
		}
	}

	void reduceMember(const std::vector<SemValue>& hijos, SemValue& resultado)
	{
		const Token& atributo = hijos[2].token;

		resultado.node = makeNode("Member", hijos[1].token.position);
		resultado.node->addChild(hijos[0].node);
		resultado.node->addChild(makeNode("Id " + atributo.lexeme, atributo.position));
	}

	void reduceIndex(const std::vector<SemValue>& hijos, SemValue& resultado)
	{
		resultado.node = makeNode("Index", hijos[1].token.position);
		resultado.node->addChild(hijos[0].node);
		resultado.node->addChild(hijos[2].node);
	}

	void reduceExprList(const Production& prod, const std::vector<SemValue>& hijos, SemValue& resultado)
	{
		if (prod.derivacion.size() == 1)
		{
			resultado.items.push_back(hijos[0]);
		}
		else
		{
			resultado.items = hijos[0].items;
			resultado.items.push_back(hijos[2]);
		}
	}

	static void agregarItems(const ASTPtr& nodo, const SemValue& lista)
	{
		for (const SemValue& item : lista.items)
			nodo->addChild(item.node);
	}

	std::string superclaseDe(const std::string& tipo)
	{
		SymbolEntry* clase = symbols.globalScope()->table.find(tipo);

		if (clase && clase->kind == SymbolKind::CLASS)
			return clase->superClass;

		return "";
	}

	bool esSubtipo(const std::string& a, const std::string& b)
	{
		if (a == b || b == "object")
			return true;

		for (std::string t = superclaseDe(a); !t.empty(); t = superclaseDe(t))
		{
			if (t == b)
				return true;
		}
		return false;
	}

	bool esAsignable(const std::string& desde, const std::string& hacia)
	{
		if (desde.empty() || hacia.empty())
			return true;

		if (esSubtipo(desde, hacia))
			return true;

		if (desde == "<None>")
			return !TypeRules::isSpecial(hacia);

		if (desde == "<Empty>")
			return TypeRules::isList(hacia);

		return false;
	}

	std::string unirTipos(const std::string& a, const std::string& b)
	{
		if (a.empty() || b.empty())
			return "";

		if (esAsignable(a, b))
			return b;

		if (esAsignable(b, a))
			return a;

		for (std::string t = superclaseDe(a); !t.empty(); t = superclaseDe(t))
		{
			if (esSubtipo(b, t))
				return t;
		}
		return "object";
	}

	void verificarCondicion(const SemValue& condicion, const std::string& sentencia)
	{
		if (condicion.type.empty() || condicion.type == "bool")
			return;

		reportarError(condicion.token.position, "la condicion del '" + sentencia + "' debe ser bool, no " + condicion.type);
	}

	void reduceCondicional(const std::vector<SemValue>& hijos, SemValue& resultado)
	{
		verificarCondicion(hijos[2], "if ... else");
		resultado.type = unirTipos(hijos[0].type, hijos[4].type);

		resultado.node = makeNode("IfExpr", hijos[1].token.position);
		resultado.node->addChild(hijos[2].node);
		resultado.node->addChild(hijos[0].node);
		resultado.node->addChild(hijos[4].node);
	}

	static bool esOperadorBinario(const std::string& nt)
	{
		return nt == "arith" || nt == "term" || nt == "cmp_expr" || nt == "and_expr" || nt == "or_expr";
	}

	void reduceBinario(const std::vector<SemValue>& hijos, SemValue& resultado)
	{
		const std::string& izq = hijos[0].type;
		const std::string& der = hijos[2].type;
		const Token& op = hijos[1].token;

		resultado.node = makeNode("BinOp " + op.lexeme, op.position);
		resultado.node->addChild(hijos[0].node);
		resultado.node->addChild(hijos[2].node);

		if (izq.empty() || der.empty())
			return;

		resultado.type = TypeRules::binaryResult(op.lexeme, izq, der);

		if (resultado.type.empty())
			reportarError(op.position, "el operador '" + op.lexeme + "' no se puede aplicar a " + izq + " y " + der);
	}

	void reduceUnario(const std::vector<SemValue>& hijos, SemValue& resultado)
	{
		const Token& op = hijos[0].token;
		const std::string& operando = hijos[1].type;

		resultado.node = makeNode("UnaryOp " + op.lexeme, op.position);
		resultado.node->addChild(hijos[1].node);

		if (operando.empty())
			return;

		resultado.type = TypeRules::unaryResult(op.lexeme, operando);

		if (resultado.type.empty())
			reportarError(op.position, "el operador '" + op.lexeme + "' no se puede aplicar a " + operando);
	}

	static std::string tipoDeVariable(const SymbolEntry* e)
	{
		if (e == nullptr)
			return "";

		if (e->kind == SymbolKind::FUNCTION || e->kind == SymbolKind::METHOD || e->kind == SymbolKind::CLASS)
			return "";

		return e->type;
	}

	static std::string tipoDeLlamada(const SymbolEntry* e)
	{
		if (e == nullptr)
			return "";

		if (e->kind == SymbolKind::FUNCTION)
			return e->type;

		if (e->kind == SymbolKind::CLASS)
			return e->name;

		return "";
	}

	SymbolEntry* usarNombre(const Token& id)
	{
		SymbolEntry* entrada = symbols.lookup(id.lexeme);
		if (entrada != nullptr)
			return entrada;

		pendientes.push_back(UsoPendiente{ id.lexeme, symbols.currentScope(), id.position });
		return nullptr;
	}

	SymbolEntry* asignarNombre(const Token& id)
	{
		SymbolEntry* local = symbols.lookupLocal(id.lexeme);

		if (local == nullptr)
		{
			reportarError(id.position, "no se puede asignar a '" + id.lexeme +
				"': no esta declarado en este scope (falta declararlo o usar global/nonlocal)");
			return nullptr;
		}

		if (local->kind == SymbolKind::FUNCTION || local->kind == SymbolKind::CLASS)
		{
			reportarError(id.position, "'" + id.lexeme + "' no es una variable, no se le puede asignar");
			return nullptr;
		}

		return local;
	}

	void reduceAssignList(const Production& prod, const std::vector<SemValue>& hijos, SemValue& resultado)
	{
		if (prod.derivacion.size() == 2)
		{
			resultado.items.push_back(hijos[0]);
		}
		else
		{
			resultado.items = hijos[0].items;
			resultado.items.push_back(hijos[1]);
		}
	}

	void verificarAsignacion(const SemValue& destinos, const SemValue& valor)
	{
		for (const SemValue& destino : destinos.items)
		{
			if (!esAsignable(valor.type, destino.type))
				reportarError(destino.token.position, "no se puede asignar un valor de tipo " + valor.type +
					" a '" + destino.token.lexeme + "' de tipo " + destino.type);
		}
	}

	void resolverPendientes()
	{
		for (const UsoPendiente& uso : pendientes)
		{
			if (symbols.lookup(uso.name, uso.scope) == nullptr)
				reportarError(uso.position, "'" + uso.name + "' no esta definido");
		}
		pendientes.clear();
	}

	void verificarTabla()
	{
		int total = static_cast<int>(symbols.getScopes().size());

		for (int id = 0; id < total; id++)
		{
			Scope* scope = symbols.getScope(id);

			for (HashTable<SymbolEntry>::Record& rec : scope->table.getRecords())
				verificarEntrada(scope, rec.value);
		}
	}

	void verificarEntrada(Scope* scope, SymbolEntry& e)
	{
		if (e.position.row <= 0)
			return;

		if (e.kind == SymbolKind::GLOBAL_REF)
			verificarGlobal(e);
		else if (e.kind == SymbolKind::NONLOCAL_REF)
			verificarNonlocal(scope, e);
		else
		{
			if (scope->kind == ScopeKind::FUNCTION)
				verificarNoTapaClase(e);

			if (e.kind != SymbolKind::CLASS)
				verificarTipo(e.type, e.position);
		}
	}

	void verificarGlobal(SymbolEntry& ref)
	{
		SymbolEntry* destino = symbols.globalScope()->table.find(ref.name);

		if (destino == nullptr || destino->kind != SymbolKind::VARIABLE)
		{
			reportarError(ref.position, "'" + ref.name + "' no es una variable global");
			return;
		}

		ref.type = destino->type;
	}

	void verificarNonlocal(Scope* scope, SymbolEntry& ref)
	{
		for (Scope* s = scope->parent; s != nullptr && s->kind == ScopeKind::FUNCTION; s = s->parent)
		{
			SymbolEntry* destino = s->table.find(ref.name);
			if (destino == nullptr)
				continue;

			if (destino->kind == SymbolKind::VARIABLE || destino->kind == SymbolKind::PARAMETER)
			{
				ref.type = destino->type;
				return;
			}
			break;
		}

		reportarError(ref.position, "'" + ref.name + "' no es una variable de una funcion que encierre a esta");
	}

	void verificarNoTapaClase(const SymbolEntry& e)
	{
		SymbolEntry* global = symbols.globalScope()->table.find(e.name);

		if (global && global->kind == SymbolKind::CLASS)
			reportarError(e.position, "'" + e.name + "' no puede usarse como nombre: tapa a la clase del mismo nombre");
	}

	void verificarTipo(const std::string& tipo, const Position& pos)
	{
		if (tipo.empty() || tipo == "<None>")
			return;

		std::string base = tipo;
		while (base.size() >= 2 && base.front() == '[' && base.back() == ']')
			base = base.substr(1, base.size() - 2);

		SymbolEntry* clase = symbols.globalScope()->table.find(base);

		if (clase == nullptr || clase->kind != SymbolKind::CLASS)
			reportarError(pos, "tipo invalido '" + tipo + "': no existe una clase llamada '" + base + "'");
	}

	void agregarParametro(SemValue& lista, const SemValue& typedVar)
	{
		lista.paramNames.push_back(typedVar.token.lexeme);
		lista.paramTypes.push_back(typedVar.type);
		lista.paramPositions.push_back(typedVar.token.position);
	}

	void copiarParametros(SemValue& destino, const SemValue& origen)
	{
		destino.paramNames = origen.paramNames;
		destino.paramTypes = origen.paramTypes;
		destino.paramPositions = origen.paramPositions;
	}

	static std::string quitarComillas(const std::string& s)
	{
		if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
			return s.substr(1, s.size() - 2);
		return s;
	}

	void reportarError(const Position& pos, const std::string& mensaje)
	{
		if (errorManager)
			errorManager->report(ErrorKind::SEMANTIC, pos, mensaje);
	}

	SymbolEntry* declarar(const SymbolEntry& entrada)
	{
		SymbolEntry* nueva = symbols.insert(entrada);
		if (nueva)
			return nueva;

		SymbolEntry* previa = symbols.lookupLocal(entrada.name);
		std::string mensaje = "'" + entrada.name + "' ya fue declarado en este scope";

		if (previa && previa->position.row > 0)
			mensaje += " (linea " + std::to_string(previa->position.row) + ")";
		else
			mensaje += " (es un nombre predefinido)";

		reportarError(entrada.position, mensaje);
		return nullptr;
	}

	void printReduce(const Production& prod, const std::vector<SemValue>& hijos)
	{
		std::cout << "REDUCE " << prod.toString() << "\n";

		for (size_t i = 0; i < hijos.size(); i++)
		{
			const SemValue& h = hijos[i];
			std::cout << "    $" << (i + 1) << " = " << h.symbol;

			if (h.symbol != h.token.lexeme)
				std::cout << " '" << h.token.lexeme << "'";

			std::cout << " (" << h.token.position.row << ":" << h.token.position.col << ")";

			if (!h.type.empty())
				std::cout << " tipo=" << h.type;

			if (!h.paramNames.empty())
				std::cout << " params=" << h.paramNames.size();

			if (h.isError)
				std::cout << " [ERROR]";

			std::cout << "\n";
		}
	}
};

#endif