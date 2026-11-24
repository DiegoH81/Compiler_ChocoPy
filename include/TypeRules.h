#ifndef TYPERULES_H
#define TYPERULES_H

#include <string>
#include <vector>

struct OperatorRule
{
	std::string op;
	std::string left;
	std::string right;
	std::string result;
};

class TypeRules
{
public:
	static const std::vector<OperatorRule>& binaryTable()
	{
		static const std::vector<OperatorRule> tabla = {
			{ "+",  "int", "int", "int" },
			{ "-",  "int", "int", "int" },
			{ "*",  "int", "int", "int" },
			{ "//", "int", "int", "int" },
			{ "%",  "int", "int", "int" },

			{ "+",  "str", "str", "str" },

			{ "<",  "int", "int", "bool" },
			{ "<=", "int", "int", "bool" },
			{ ">",  "int", "int", "bool" },
			{ ">=", "int", "int", "bool" },
			{ "==", "int", "int", "bool" },
			{ "!=", "int", "int", "bool" },

			{ "==", "bool", "bool", "bool" },
			{ "!=", "bool", "bool", "bool" },

			{ "==", "str", "str", "bool" },
			{ "!=", "str", "str", "bool" },

			{ "and", "bool", "bool", "bool" },
			{ "or",  "bool", "bool", "bool" }
		};
		return tabla;
	}

	static const std::vector<OperatorRule>& unaryTable()
	{
		static const std::vector<OperatorRule> tabla = {
			{ "-",   "", "int",  "int" },
			{ "not", "", "bool", "bool" }
		};
		return tabla;
	}

	static std::string binaryResult(const std::string& op, const std::string& left, const std::string& right)
	{
		for (const OperatorRule& regla : binaryTable())
		{
			if (regla.op == op && regla.left == left && regla.right == right)
				return regla.result;
		}

		if (op == "is" && !isSpecial(left) && !isSpecial(right))
			return "bool";

		return "";
	}

	static std::string unaryResult(const std::string& op, const std::string& operand)
	{
		for (const OperatorRule& regla : unaryTable())
		{
			if (regla.op == op && regla.right == operand)
				return regla.result;
		}
		return "";
	}

	static bool isList(const std::string& tipo)
	{
		return tipo.size() >= 2 && tipo.front() == '[' && tipo.back() == ']';
	}

	static bool isSpecial(const std::string& tipo)
	{
		return tipo == "int" || tipo == "bool" || tipo == "str";
	}
};

#endif
