#ifndef ASTDRAWER_H
#define ASTDRAWER_H

#include <inja/inja.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>

#include "ASTNode.h"
#include "ASTTraversal.h"

class ASTDrawer
{
public:
	ASTDrawer(std::filesystem::path inBaseDir) :
		baseDir(inBaseDir)
	{}

	void saveToFile(const std::string& filename, const std::string& sourceName, const ASTPtr& raiz)
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

		file << generateHTML(sourceName, raiz);
		file.close();

		std::cout << "AST guardado en: " << finalPath << "\n";
	}

private:
	std::filesystem::path baseDir;

	std::string generateHTML(const std::string& sourceName, const ASTPtr& raiz)
	{
		std::vector<ASTPtr> todos;
		ASTTraversal::preorder(raiz, todos);

		int hojas = 0;
		for (const ASTPtr& nodo : todos)
		{
			if (nodo->isLeaf())
				hojas++;
		}

		nlohmann::json data;
		data["source"] = escapeHtml(sourceName);
		data["tree"] = dibujarNodo(raiz, true);
		data["totalNodes"] = static_cast<int>(todos.size());
		data["leaves"] = hojas;
		data["internal"] = static_cast<int>(todos.size()) - hojas;
		data["depth"] = profundidad(raiz);
		data["statements"] = nlohmann::json::array();

		for (const ASTPtr& sentencia : raiz->children)
		{
			std::vector<ASTPtr> recorrido;
			ASTTraversal::preorder(sentencia, recorrido);

			nlohmann::json fila;
			fila["line"] = sentencia->position.row;
			fila["nodes"] = nlohmann::json::array();

			for (const ASTPtr& nodo : recorrido)
			{
				nlohmann::json n;
				n["label"] = escapeHtml(nodo->label);
				n["leaf"] = nodo->isLeaf();
				fila["nodes"].push_back(n);
			}

			data["statements"].push_back(fila);
		}

		inja::Environment env;
		std::string templatePath = std::string(CHOCOPY_ROOT) + "/templates/ast_template.html";
		return env.render_file(templatePath, data);
	}

	std::string dibujarNodo(const ASTPtr& nodo, bool esRaiz)
	{
		std::string clase = esRaiz ? "root" : (nodo->isLeaf() ? "leaf" : "op");

		if (!nodo->isLeaf())
			clase += " toggle";

		std::string html = "<li><div class=\"node " + clase + "\" title=\"linea " +
			std::to_string(nodo->position.row) + ", columna " + std::to_string(nodo->position.col) + "\">";

		html += "<span class=\"label\">" + escapeHtml(nodo->label) + "</span>";

		if (!nodo->type.empty())
			html += "<span class=\"type\">" + escapeHtml(nodo->type) + "</span>";

		html += "</div>";

		if (!nodo->isLeaf())
		{
			html += "<ul>";
			for (const ASTPtr& hijo : nodo->children)
				html += dibujarNodo(hijo, false);
			html += "</ul>";
		}

		return html + "</li>";
	}

	static int profundidad(const ASTPtr& nodo)
	{
		if (!nodo)
			return 0;

		int maxHijo = 0;
		for (const ASTPtr& hijo : nodo->children)
		{
			int p = profundidad(hijo);
			if (p > maxHijo)
				maxHijo = p;
		}
		return maxHijo + 1;
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
