#ifndef ASTTRAVERSAL_H
#define ASTTRAVERSAL_H

#include <iostream>
#include <string>
#include <vector>

#include "ASTNode.h"

class ASTTraversal
{
public:
	static void preorder(const ASTPtr& nodo, std::vector<ASTPtr>& salida)
	{
		if (!nodo)
			return;

		salida.push_back(nodo);

		for (const ASTPtr& hijo : nodo->children)
			preorder(hijo, salida);
	}

	static void postorder(const ASTPtr& nodo, std::vector<ASTPtr>& salida)
	{
		if (!nodo)
			return;

		for (const ASTPtr& hijo : nodo->children)
			postorder(hijo, salida);

		salida.push_back(nodo);
	}

	static void printTree(const ASTPtr& nodo, std::ostream& os, int nivel = 0)
	{
		if (!nodo)
			return;

		os << std::string(nivel * 4, ' ') << nodo->label;

		if (!nodo->type.empty())
			os << "  : " << nodo->type;

		os << "\n";

		for (const ASTPtr& hijo : nodo->children)
			printTree(hijo, os, nivel + 1);
	}

	static std::string toLine(const std::vector<ASTPtr>& nodos)
	{
		std::string linea;

		for (size_t i = 0; i < nodos.size(); i++)
		{
			if (i > 0)
				linea += "  ";
			linea += nodos[i]->label;
		}
		return linea;
	}

	static void printRecorridos(const ASTPtr& raiz, std::ostream& os)
	{
		os << "\n========== AST (DFS) ==========\n";
		printTree(raiz, os);

		for (const ASTPtr& sentencia : raiz->children)
		{
			std::vector<ASTPtr> pre;
			preorder(sentencia, pre);

			os << "\n[linea " << sentencia->position.row << "]\n"
			   << "  preorden:  " << toLine(pre) << "\n";
		}
	}
};

#endif
