#ifndef SEMANTICVALUE_H
#define SEMANTICVALUE_H

#include <string>
#include <vector>

#include "Token.h"
#include "ASTNode.h"

struct SemValue
{
	std::string symbol;
	Token token;
	bool isError = false;

	std::string type;
	std::vector<std::string> paramNames;
	std::vector<std::string> paramTypes;
	std::vector<Position> paramPositions;

	std::vector<SemValue> items;

	ASTPtr node;

	SemValue(const std::string& inSymbol, const Token& inToken, bool inIsError = false) :
		symbol(inSymbol), token(inToken), isError(inIsError)
	{}
};

#endif