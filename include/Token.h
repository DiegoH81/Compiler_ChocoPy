#ifndef TOKEN_H
#define TOKEN_H

#include <iostream>
#include <string>
#include <iomanip>

#include "Position.h"

enum TokenType
{
	// Physical lines
	EOFL,
	EOL,

	// Logical lines
	NEW_LINE,

	// Comments
	HASHTAG,

	// Indentation
	INDENT,
	DEDENT,

	// Identifiers
	IDENTIFIER,

	// Literals
	INTEGER,
	STRING,

	// Keywords
	KEYWORD_FALSE,
	KEYWORD_NONE,
	KEYWORD_TRUE,
	KEYWORD_AND,
	KEYWORD_AS,
	KEYWORD_ASSERT,
	KEYWORD_ASYNC,
	KEYWORD_AWAIT,
	KEYWORD_BREAK,
	KEYWORD_CLASS,
	KEYWORD_CONTINUE,
	KEYWORD_DEF,
	KEYWORD_DEL,
	KEYWORD_ELIF,
	KEYWORD_ELSE,
	KEYWORD_EXCEPT,
	KEYWORD_FINALLY,
	KEYWORD_FOR,
	KEYWORD_FROM,
	KEYWORD_GLOBAL,
	KEYWORD_IF,
	KEYWORD_IMPORT,
	KEYWORD_IN,
	KEYWORD_IS,
	KEYWORD_LAMBDA,
	KEYWORD_NONLOCAL,
	KEYWORD_NOT,
	KEYWORD_OR,
	KEYWORD_PASS,
	KEYWORD_RAISE,
	KEYWORD_RETURN,
	KEYWORD_TRY,
	KEYWORD_WHILE,
	KEYWORD_WITH,
	KEYWORD_YIELD,

	// Delimiter
	DELIM_LPAREN,
	DELIM_RPAREN,
	DELIM_LBRACKET,
	DELIM_RBRACKET,
	DELIM_COMMA,
	DELIM_SEMICOLON,
	DELIM_COLON,
	DELIM_DOT,
	OP_ARROW,

	// Operator
	OP_PLUS,
	OP_MINUS,
	OP_STAR,
	OP_SLASH_SLASH,
	OP_PERCENT,
	OP_LT,
	OP_GT,
	OP_LE,
	OP_GE,
	OP_EQEQ,
	OP_NEQ,
	OP_ASSIGN,

	// Error
	UNRECOGNIZED
};

class Token
{
public:
	TokenType token;
	std::string lexeme;
	Position position;

	Token(TokenType inToken, const std::string& inLexeme, const Position& inPos):
		token(inToken), lexeme(inLexeme), position(inPos)
	{}

	friend std::ostream& operator << (std::ostream& os, const Token& t)
	{
		os << std::left
			<< std::setw(15) << tokenTypeToStream(t.token)
			<< "[ "
			<< std::setw(15) << t.lexeme
			<< "] (" << t.position.row << ":" << t.position.col << ")";

		return os;
	}
private:
	static std::string tokenTypeToStream(TokenType inToken)
	{
		switch (inToken)
		{
		case EOFL: return "EOFL";
		case NEW_LINE: return "NEW_LINE";
		case HASHTAG: return "HASHTAG";
		case INDENT: return "INDENT";
		case DEDENT: return "DEDENT";
		case IDENTIFIER: return "IDENTIFIER";
		case INTEGER: return "INTEGER";
		case STRING: return "STRING";
		case KEYWORD_FALSE: return "KEYWORD_FALSE";
		case KEYWORD_NONE: return "KEYWORD_NONE";
		case KEYWORD_TRUE: return "KEYWORD_TRUE";
		case KEYWORD_AND: return "KEYWORD_AND";
		case KEYWORD_AS: return "KEYWORD_AS";
		case KEYWORD_ASSERT: return "KEYWORD_ASSERT";
		case KEYWORD_ASYNC: return "KEYWORD_ASYNC";
		case KEYWORD_AWAIT: return "KEYWORD_AWAIT";
		case KEYWORD_BREAK: return "KEYWORD_BREAK";
		case KEYWORD_CLASS: return "KEYWORD_CLASS";
		case KEYWORD_CONTINUE: return "KEYWORD_CONTINUE";
		case KEYWORD_DEF: return "KEYWORD_DEF";
		case KEYWORD_DEL: return "KEYWORD_DEL";
		case KEYWORD_ELIF: return "KEYWORD_ELIF";
		case KEYWORD_ELSE: return "KEYWORD_ELSE";
		case KEYWORD_EXCEPT: return "KEYWORD_EXCEPT";
		case KEYWORD_FINALLY: return "KEYWORD_FINALLY";
		case KEYWORD_FOR: return "KEYWORD_FOR";
		case KEYWORD_FROM: return "KEYWORD_FROM";
		case KEYWORD_GLOBAL: return "KEYWORD_GLOBAL";
		case KEYWORD_IF: return "KEYWORD_IF";
		case KEYWORD_IMPORT: return "KEYWORD_IMPORT";
		case KEYWORD_IN: return "KEYWORD_IN";
		case KEYWORD_IS: return "KEYWORD_IS";
		case KEYWORD_LAMBDA: return "KEYWORD_LAMBDA";
		case KEYWORD_NONLOCAL: return "KEYWORD_NONLOCAL";
		case KEYWORD_NOT: return "KEYWORD_NOT";
		case KEYWORD_OR: return "KEYWORD_OR";
		case KEYWORD_PASS: return "KEYWORD_PASS";
		case KEYWORD_RAISE: return "KEYWORD_RAISE";
		case KEYWORD_RETURN: return "KEYWORD_RETURN";
		case KEYWORD_TRY: return "KEYWORD_TRY";
		case KEYWORD_WHILE: return "KEYWORD_WHILE";
		case KEYWORD_WITH: return "KEYWORD_WITH";
		case KEYWORD_YIELD: return "KEYWORD_YIELD";
		case DELIM_LPAREN: return "DELIM_LPAREN";
		case DELIM_RPAREN: return "DELIM_RPAREN";
		case DELIM_LBRACKET: return "DELIM_LBRACKET";
		case DELIM_RBRACKET: return "DELIM_RBRACKET";
		case DELIM_COMMA: return "DELIM_COMMA";
		case DELIM_COLON: return "DELIM_COLON";
		case DELIM_SEMICOLON: return "DELIM_SEMICOLON";
		case DELIM_DOT: return "DELIM_DOT";
		case OP_PLUS: return "OP_PLUS";
		case OP_MINUS: return "OP_MINUS";
		case OP_STAR: return "OP_STAR";
		case OP_SLASH_SLASH: return "OP_SLASH_SLASH";
		case OP_PERCENT: return "OP_PERCENT";
		case OP_LT: return "OP_LT";
		case OP_GT: return "OP_GT";
		case OP_LE: return "OP_LE";
		case OP_GE: return "OP_GE";
		case OP_EQEQ: return "OP_EQEQ";
		case OP_NEQ: return "OP_NEQ";
		case OP_ASSIGN: return "OP_ASSIGN";
		case OP_ARROW: return "OP_ARROW";
		default: return "UNKNOWN_TOKEN";
		}
	}
};

#endif