#ifndef ERRORMANAGER_H
#define ERRORMANAGER_H

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "Position.h"

enum class ErrorKind
{
	LEXICAL,
	SYNTACTIC,
	SEMANTIC
};

struct CompilerError
{
	ErrorKind kind;
	Position position;
	std::string message;
};

class ErrorManager
{
public:
	void report(ErrorKind kind, const Position& pos, const std::string& message)
	{
		errors.push_back({ kind, pos, message });
	}

	void clear()
	{
		errors.clear();
	}

	int count(ErrorKind kind) const
	{
		int n = 0;
		for (const CompilerError& e : errors)
			if (e.kind == kind)
				n++;
		return n;
	}

	int total() const
	{
		return static_cast<int>(errors.size());
	}

	bool hasErrors() const
	{
		return !errors.empty();
	}

	const std::vector<CompilerError>& getErrors() const
	{
		return errors;
	}

	void printSummary(std::ostream& os = std::cout) const
	{
		os << "\n========== ERRORES ==========\n";

		if (errors.empty())
			os << "Sin errores.\n";

		std::vector<CompilerError> ordenados = errors;
		std::stable_sort(ordenados.begin(), ordenados.end(), vaAntes);

		for (const CompilerError& e : ordenados)
		{
			os << "[" << kindToString(e.kind) << "] ("
			   << e.position.row << ":" << e.position.col << ") "
			   << e.message << "\n";
		}

		os << "-----------------------------\n"
		   << "Lexicos:     " << count(ErrorKind::LEXICAL) << "\n"
		   << "Sintacticos: " << count(ErrorKind::SYNTACTIC) << "\n"
		   << "Semanticos:  " << count(ErrorKind::SEMANTIC) << "\n"
		   << "TOTAL:       " << total() << "\n";
	}

	static std::string kindToString(ErrorKind kind)
	{
		switch (kind)
		{
		case ErrorKind::LEXICAL:   return "LEXICO";
		case ErrorKind::SYNTACTIC: return "SINTACTICO";
		case ErrorKind::SEMANTIC:  return "SEMANTICO";
		}
		return "?";
	}

private:
	std::vector<CompilerError> errors;

	static bool vaAntes(const CompilerError& a, const CompilerError& b)
	{
		if (a.position.row != b.position.row)
			return a.position.row < b.position.row;
		return a.position.col < b.position.col;
	}
};

#endif