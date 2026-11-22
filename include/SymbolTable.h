#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <string>
#include <vector>
#include <deque>
#include <iostream>

#include "HashTable.h"
#include "Position.h"

enum class SymbolKind
{
	VARIABLE,
	PARAMETER,
	FUNCTION,
	CLASS,
	ATTRIBUTE,
	METHOD,
	GLOBAL_REF,
	NONLOCAL_REF
};

inline std::string kindToString(SymbolKind kind)
{
	switch (kind)
	{
	case SymbolKind::VARIABLE:     return "variable";
	case SymbolKind::PARAMETER:    return "parametro";
	case SymbolKind::FUNCTION:     return "funcion";
	case SymbolKind::CLASS:        return "clase";
	case SymbolKind::ATTRIBUTE:    return "atributo";
	case SymbolKind::METHOD:       return "metodo";
	case SymbolKind::GLOBAL_REF:   return "global";
	case SymbolKind::NONLOCAL_REF: return "nonlocal";
	}
	return "?";
}

enum class ScopeKind
{
	GLOBAL,
	FUNCTION,
	CLASS
};

inline std::string scopeKindToString(ScopeKind kind)
{
	switch (kind)
	{
	case ScopeKind::GLOBAL:   return "global";
	case ScopeKind::FUNCTION: return "funcion";
	case ScopeKind::CLASS:    return "clase";
	}
	return "?";
}

struct SymbolEntry
{
	std::string name;
	SymbolKind kind;
	std::string type;
	Position position;

	std::vector<std::string> paramNames;
	std::vector<std::string> paramTypes;
	std::string superClass;
	int bodyScopeId = -1;
};

struct Scope
{
	int id;
	std::string name;
	ScopeKind kind;
	Scope* parent;
	int level;
	HashTable<SymbolEntry> table;
};

class SymbolTable
{
public:
	SymbolTable()
	{
		reset();
	}

	void reset()
	{
		scopes.clear();
		current = nullptr;
		openScope("global", ScopeKind::GLOBAL);
		insertPredefined();
	}

	Scope* openScope(const std::string& name, ScopeKind kind)
	{
		int id = static_cast<int>(scopes.size());
		int level = current ? current->level + 1 : 0;

		scopes.push_back(Scope{ id, name, kind, current, level, HashTable<SymbolEntry>() });
		current = &scopes.back();
		return current;
	}

	void closeScope()
	{
		if (current && current->parent)
			current = current->parent;
	}

	SymbolEntry* insert(const SymbolEntry& entry)
	{
		return current->table.insert(entry.name, entry);
	}

	SymbolEntry* lookup(const std::string& name, Scope* desde = nullptr)
	{
		Scope* s = lookupScope(name, desde);
		return s ? s->table.find(name) : nullptr;
	}

	SymbolEntry* lookupLocal(const std::string& name)
	{
		return current->table.find(name);
	}

	Scope* lookupScope(const std::string& name, Scope* desde = nullptr)
	{
		Scope* s = desde ? desde : current;

		for (; s != nullptr; s = s->parent)
		{
			if (s->kind == ScopeKind::CLASS)
				continue;

			if (s->table.contains(name))
				return s;
		}
		return nullptr;
	}

	Scope* currentScope()
    {
        return current;
    }
	Scope* globalScope()

    {
        return &scopes.front();
    }

	Scope* getScope(int id)
    {
        return &scopes[id];
    }

	const std::deque<Scope>& getScopes() const
    {
        return scopes;
    }

	void print(std::ostream& os = std::cout) const
	{
		os << "\n========== TABLA DE SIMBOLOS ==========\n";

		for (const Scope& s : scopes)
		{
			os << "Scope " << s.id << ": " << s.name
			   << " (nivel " << s.level << ", padre "
			   << (s.parent ? std::to_string(s.parent->id) : "-") << ")\n";

			for (const auto& rec : s.table.getRecords())
			{
				const SymbolEntry& e = rec.value;
				os << "    " << e.name << " : " << kindToString(e.kind)
				   << " : " << e.type
				   << " (" << e.position.row << ":" << e.position.col << ")\n";

				if (e.kind == SymbolKind::CLASS && !e.superClass.empty())
					os << "        hereda de " << e.superClass << "\n";
			}
		}
	}

private:
	std::deque<Scope> scopes;
	Scope* current = nullptr;

	void insertPredefined()
	{
		Position none(0, 0);

		insert(SymbolEntry{ "object", SymbolKind::CLASS, "object", none, {}, {}, "" });
		insert(SymbolEntry{ "int", SymbolKind::CLASS, "int", none, {}, {}, "object" });
		insert(SymbolEntry{ "bool", SymbolKind::CLASS, "bool", none, {}, {}, "object" });
		insert(SymbolEntry{ "str", SymbolKind::CLASS, "str", none, {}, {}, "object" });

		insert(SymbolEntry{ "print", SymbolKind::FUNCTION, "<None>", none, { "arg" }, { "object" } });
		insert(SymbolEntry{ "input", SymbolKind::FUNCTION, "str", none, {}, {} });
		insert(SymbolEntry{ "len", SymbolKind::FUNCTION, "int", none, { "arg" }, { "object" } });
	}
};

#endif