#ifndef SYMBOLTABLEDRAWER_H
#define SYMBOLTABLEDRAWER_H

#include <inja/inja.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>

#include "SymbolTable.h"

class SymbolTableDrawer
{
public:
	SymbolTableDrawer(std::filesystem::path inBaseDir) :
		baseDir(inBaseDir)
	{}

	void saveToFile(const std::string& filename, const std::string& sourceName, const SymbolTable& symbols)
	{
		std::filesystem::path dir = baseDir / "tables";

		if (!std::filesystem::exists(dir))
			std::filesystem::create_directories(dir);

		std::filesystem::path finalPath = dir / filename;
		std::ofstream file(finalPath.string());

		if (!file.is_open())
		{
			std::cerr << "ERROR: no se pudo guardar el HTML en: " << finalPath << "\n";
			return;
		}

		file << generateHTML(sourceName, symbols);
		file.close();

		std::cout << "Tabla de simbolos guardada en: " << finalPath << "\n";
	}

private:
	std::filesystem::path baseDir;

	std::string generateHTML(const std::string& sourceName, const SymbolTable& symbols)
	{
		nlohmann::json data;
		data["source"] = escapeHtml(sourceName);
		data["scopes"] = nlohmann::json::array();

		for (const Scope& s : symbols.getScopes())
		{
			nlohmann::json scope;
			scope["id"] = s.id;
			scope["name"] = escapeHtml(s.name);
			scope["kind"] = scopeKindToString(s.kind);
			scope["level"] = s.level;
			scope["parent"] = s.parent ? std::to_string(s.parent->id) : "-";
			scope["entries"] = nlohmann::json::array();

			for (const HashTable<SymbolEntry>::Record& rec : s.table.getRecords())
				scope["entries"].push_back(entryToJson(rec.value));

			data["scopes"].push_back(scope);
		}

		inja::Environment env;
		std::string templatePath = std::string(CHOCOPY_ROOT) + "/templates/symbols_template.html";
		return env.render_file(templatePath, data);
	}

	nlohmann::json entryToJson(const SymbolEntry& e)
	{
		nlohmann::json r;
		r["name"] = escapeHtml(e.name);
		r["kind"] = kindToString(e.kind);
		r["type"] = escapeHtml(e.type);
		r["detail"] = escapeHtml(detalle(e));
		r["position"] = std::to_string(e.position.row) + ":" + std::to_string(e.position.col);
		r["predefined"] = e.position.row <= 0;
		return r;
	}

	static std::string detalle(const SymbolEntry& e)
	{
		std::string texto;

		if (e.kind == SymbolKind::CLASS && !e.superClass.empty())
			texto = "hereda de " + e.superClass;
		else if (e.kind == SymbolKind::FUNCTION || e.kind == SymbolKind::METHOD)
			texto = firma(e);

		if (e.bodyScopeId >= 0)
			texto += "  (cuerpo: scope " + std::to_string(e.bodyScopeId) + ")";

		return texto;
	}

	static std::string firma(const SymbolEntry& e)
	{
		std::string texto = "(";

		for (size_t i = 0; i < e.paramTypes.size(); i++)
		{
			if (i > 0)
				texto += ", ";
			texto += e.paramNames[i] + ": " + e.paramTypes[i];
		}

		return texto + ") -> " + e.type;
	}

	static std::string escapeHtml(const std::string& s)
	{
		std::string out;

		for (char c : s)
		{
			if (c == '&')
				out += "&amp;";
			else if (c == '<')
				out += "&lt;";
			else if (c == '>')
				out += "&gt;";
			else if (c == '"')
				out += "&quot;";
			else
				out += c;
		}
		return out;
	}
};

#endif
