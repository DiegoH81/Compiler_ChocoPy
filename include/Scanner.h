#ifndef SCANNER_H
#define SCANNER_H

#include <map>
#include <string>
#include <sstream>
#include <fstream>

#include "Token.h"

class Scanner
{
public:
	Scanner(const std::string& sourcePath)
	{
		loadFile(sourcePath);
		initKeywords();
		initOperators();

		std::cout << buffer.size() << "\n";
	}

	Token getToken()
	{

	}
private:
	int buff_pos;
	std::string buffer;

	std::map<std::string, TokenType> physicalLines, keywords, operators;

	void loadFile(const std::string& source)
	{
		std::ifstream file(source);
		if (!file)
		{
			std::cerr << "SCANNER: File not found! " + source << "\n";
			return;
		}

		std::stringstream bufferSS;
		bufferSS << file.rdbuf();

		buffer = bufferSS.str();

		file.close();
	}

	void initKeywords()
	{
		keywords["false"] = KEYWORD_FALSE;
		keywords["none"] = KEYWORD_NONE;
		keywords["true"] = KEYWORD_TRUE;
		keywords["and"] = KEYWORD_AND;
		keywords["def"] = KEYWORD_DEF;
		keywords["if"] = KEYWORD_IF;
		keywords["else"] = KEYWORD_ELSE;
		keywords["return"] = KEYWORD_RETURN;
		keywords["while"] = KEYWORD_WHILE;
	}

	void initOperators()
	{
		operators["+"] = OP_PLUS;
		operators["-"] = OP_MINUS;
		operators["*"] = OP_STAR;
		operators["//"] = OP_SLASH_SLASH;
		operators["%"] = OP_PERCENT;
		operators["<"] = OP_LT;
		operators[">"] = OP_GT;
		operators["<="] = OP_LE;
		operators[">="] = OP_GE;
		operators["=="] = OP_EQEQ;
		operators["!="] = OP_NEQ;
		operators["="] = OP_ASSIGN;
		operators["->"] = OP_ARROW;
		operators["("] = DELIM_LPAREN;
		operators[")"] = DELIM_RPAREN;
		operators["["] = DELIM_LBRACKET;
		operators["]"] = DELIM_RBRACKET;
		operators[","] = DELIM_COMMA;
		operators[":"] = DELIM_COLON;
		operators["."] = DELIM_DOT;
	}

	char getChar()
	{

	}

	char peekChar()
	{

	}
};

#endif