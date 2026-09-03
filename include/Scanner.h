#ifndef SCANNER_H
#define SCANNER_H

#include <unordered_map>
#include <string>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <stack>
#include <queue>

#include "Token.h"
#include "Position.h"

class Scanner
{
public:
	Scanner() :
		buffer(), buffPos(0), isNewLine(true), spacesStack(),
		currentPos(1, 1), errorCounter(0), pendingTokens()
	{
		initKeywords();

		spacesStack.push(0);
	}

	void loadOneFile(const std::string& sourcePath)
	{
		buffPos = 0;
		isNewLine = true;
		spacesStack = std::stack<int>();
		spacesStack.push(0);

		currentPos.row = currentPos.col = 1;
		errorCounter = 0;
		pendingTokens = std::queue<Token>();
		buffer.clear();


		loadFile(sourcePath);
	}

	void scanWholeThing()
	{
		if (buffer.empty())
		{
			std::cerr << "File not initialized, can't SCAN!\n";
			return;
		}

		std::cout << "INFO SCANNER - Start scanning\n";

		while (true)
		{
			auto token = getToken();
			std::cout << "DEBUG SCAN - " << token << "\n";


			if (token.token == TokenType::EOFL)
			{
				std::cout << "INFO SCANNER - Completed with " << errorCounter << " errors\n";
				return;;
			}
		}
	}

	void bulkScan(const std::string folderPath)
	{
		std::vector<std::string> fileList;
		std::filesystem::path baseDir(folderPath);

		for (const auto& entry : std::filesystem::directory_iterator(baseDir))
		{
			if (entry.is_regular_file())
			{
				std::filesystem::path fullPath = baseDir / entry.path().filename();
				fileList.push_back(fullPath.string());
			}
		}

		std::cout << "INITIALIZING BULK LOAD\n";
		for (auto& f : fileList)
		{
			std::cout << "Scanning file: " << f << "\n";
			buffer.clear();
			loadOneFile(f);

			scanWholeThing();

			std::cout << "\n\n\n";
		}

	}

	Token getToken()
	{
		if (!pendingTokens.empty())
		{
			Token top = pendingTokens.front();
			pendingTokens.pop();

			return top;
		}

		if (isNewLine)
		{
			isNewLine = false;
			processIndentation();

			if (!pendingTokens.empty())
			{
				Token top = pendingTokens.front();
				pendingTokens.pop();

				return top;
			}
		}

		Position startPos = currentPos;
		char c = getChar();

		while (true)
		{

		
		switch (c)
		{
		// Comments
		case '#':
		{
			while (peekChar() != '\n' && peekChar() != '\0')
				getChar();

			getChar();
			Token toReturn(TokenType::NEW_LINE, "\\n", currentPos);

			currentPos.row++;
			currentPos.col = 1;

			isNewLine = true;
			return toReturn;
		}
		// Operators
		case '+':
		{
			return Token(TokenType::OP_PLUS, "+", startPos);
			break;
		}
		case '-':
		{
			if (peekChar() == '>')
			{
				getChar();
				return Token(TokenType::OP_ARROW, "->", startPos);
			}

			return Token(TokenType::OP_MINUS, "-", startPos);
			break;
		}
		case '/':
		{
			if (peekChar() == '/')
			{
				getChar();
				return Token(TokenType::OP_SLASH_SLASH, "//", startPos);
			}

			logError(currentPos, "OPERATOR", "Unrecognized /");
			return Token(TokenType::UNRECOGNIZED, "/", startPos);
		}
		case '%':
		{
			return Token(TokenType::OP_PERCENT, "%", startPos);
			break;
		}
		case '<':
		{
			if (peekChar() == '=')
			{
				getChar();

				return Token(TokenType::OP_LE, "<=", startPos);
			}

			return Token(TokenType::OP_LT, "<", startPos);
			break;
		}
		case '>':
		{
			if (peekChar() == '=')
			{
				getChar();

				return Token(TokenType::OP_GE, ">=", startPos);
			}

			return Token(TokenType::OP_GT, ">", startPos);
			break;
		}
		case '=':
		{
			if (peekChar() == '=')
			{
				getChar();

				return Token(TokenType::OP_EQEQ, "==", startPos);
			}

			return Token(TokenType::OP_ASSIGN, "=", startPos);
			break;
		}
		case '*':
		{
			return Token(TokenType::OP_STAR, "*", startPos);
			break;
		}
		case '!':
		{
			if (peekChar() == '=')
			{
				getChar();

				return Token(TokenType::OP_NEQ, "!=", startPos);
			}

			logError(startPos, "OPERATOR", "Unrecognized !");
			return Token(TokenType::UNRECOGNIZED, "!", startPos);
			break;
		}

		// Integers
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		{
			std::string number(1, c);

			while (isNumber(peekChar()))
				number += getChar();


			if (number.length() > 1 && number[0] == '0')
			{
				logError(currentPos, "INTEGER", "Integer starting with 0");
				return Token(TokenType::UNRECOGNIZED, number, startPos);
			}


			long long value = std::stoll(number);



			if (value > INT32_MAX || value < INT32_MIN)
			{
				logError(currentPos, "INTEGER", "Out of 32 bits range");
				return Token(TokenType::UNRECOGNIZED, number, startPos);
			}

			return Token(TokenType::INTEGER, number, startPos);
		}

		// String
		case '"':
		{
			std::string str(1, c);

			while (true)
			{
				char next = peekChar();

				if (next == '\n' || next == '\0')
				{
					logError(startPos, "STRING", "Unclosed STRING, missing \"");
					return Token(TokenType::UNRECOGNIZED, str, startPos);
				}

				if (next == '"')
				{
					getChar();
					str += '"';

					return Token(TokenType::STRING, str, startPos);
				}

				if (next == '\\')
				{
					getChar();
					char esc = peekChar();

					if (esc == 'n' || esc == 't' || esc == '"' || esc == '\\')
					{
						str += '\\';
						str += getChar();
					}
					else
					{
						logError(startPos, "STRING", "Unrecognized invalida SCAPE");

						str += '\\';
						if (esc != '\0')
							str += getChar();
					}
					continue;
				}

				if (!isValidStringChar(next))
				{
					logError(startPos, "STRING", "CHAR out of range ASCII " + std::to_string((int)(unsigned char)next));
					getChar();
					continue;
				}

				str += getChar();
			}

			break;
		}
		// New line
		case '\n':
		{
			Token toReturn(TokenType::NEW_LINE, "\\n", startPos);

			currentPos.row++;
			currentPos.col = 1;
			isNewLine = true;

			return toReturn;
		}

		// End of file
		case '\0':
		{
			processDedentation();

			if (!pendingTokens.empty())
			{
				Token top = pendingTokens.front();
				pendingTokens.pop();

				pendingTokens.push(Token(TokenType::EOFL, "\\0", startPos));
				return top;
			}
			else
				return Token(TokenType::EOFL, "\\0", startPos);

		}

		// Delimiters
		case '(':
			return Token(TokenType::DELIM_LPAREN, "(", startPos);
		case ')':
			return Token(TokenType::DELIM_RPAREN, ")", startPos);
		case '[':
			return Token(TokenType::DELIM_LBRACKET, "[", startPos);
		case ']':
			return Token(TokenType::DELIM_RBRACKET, "]", startPos);
		case ',':
			return Token(TokenType::DELIM_COMMA, ",", startPos);
		case ':':
			return Token(TokenType::DELIM_COLON, ":", startPos);
		case '.':
			return Token(TokenType::DELIM_DOT, ".", startPos);
		// Space
		case ' ':
		case '\t':
		{
			while (peekChar() == ' ' || peekChar() == '\t')
				getChar();

			c = getChar();

			startPos = currentPos;
			break;
		}
		// Variable 1
		case '_':
		{
			std::string str(1, c);
			while (isAlphaNumeric(peekChar()))
				str += getChar();

			return Token(TokenType::IDENTIFIER, str, startPos);
		}
		// Variable 2
		default:
		{
			if (isAlphaLetter(c))
			{
				std::string str(1, c);

				while (isAlphaNumeric(peekChar()))
					str += getChar();

				if (keywords.find(str) != keywords.end()) // Is keyword
					return Token(keywords[str], str, startPos);
				else
					return Token(TokenType::IDENTIFIER, str, startPos);
			}

			logError(startPos, "ERROR", "Unrecognized char");
			return Token(TokenType::UNRECOGNIZED, std::string(1, c), startPos);
		}
		}
		}
	}
private:
	int buffPos, errorCounter;
	bool isNewLine;
	Position currentPos;
	std::string buffer;

	std::stack<int> spacesStack;
	std::queue<Token> pendingTokens;

	std::unordered_map<std::string, TokenType> keywords;

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
		keywords["False"] = KEYWORD_FALSE;
		keywords["None"] = KEYWORD_NONE;
		keywords["True"] = KEYWORD_TRUE;
		keywords["and"] = KEYWORD_AND;
		keywords["as"] = KEYWORD_AS;
		keywords["assert"] = KEYWORD_ASSERT;
		keywords["async"] = KEYWORD_ASYNC;
		keywords["await"] = KEYWORD_AWAIT;
		keywords["break"] = KEYWORD_BREAK;
		keywords["class"] = KEYWORD_CLASS;
		keywords["continue"] = KEYWORD_CONTINUE;
		keywords["def"] = KEYWORD_DEF;
		keywords["del"] = KEYWORD_DEL;
		keywords["elif"] = KEYWORD_ELIF;
		keywords["else"] = KEYWORD_ELSE;
		keywords["except"] = KEYWORD_EXCEPT;
		keywords["finally"] = KEYWORD_FINALLY;
		keywords["for"] = KEYWORD_FOR;
		keywords["from"] = KEYWORD_FROM;
		keywords["global"] = KEYWORD_GLOBAL;
		keywords["if"] = KEYWORD_IF;
		keywords["import"] = KEYWORD_IMPORT;
		keywords["in"] = KEYWORD_IN;
		keywords["is"] = KEYWORD_IS;
		keywords["lambda"] = KEYWORD_LAMBDA;
		keywords["nonlocal"] = KEYWORD_NONLOCAL;
		keywords["not"] = KEYWORD_NOT;
		keywords["or"] = KEYWORD_OR;
		keywords["pass"] = KEYWORD_PASS;
		keywords["raise"] = KEYWORD_RAISE;
		keywords["return"] = KEYWORD_RETURN;
		keywords["try"] = KEYWORD_TRY;
		keywords["while"] = KEYWORD_WHILE;
		keywords["with"] = KEYWORD_WITH;
		keywords["yield"] = KEYWORD_YIELD;
	}

	void processIndentation()
	{
		int spacesCount = 0;

		Position start_pos = currentPos;
		while (!isAtEnd(buffPos) && (peekChar() == ' '))
		{
			spacesCount++;
			getChar();
		}


		if (peekChar() == '\t')
		{
			logError(currentPos, "INDENT", "TAB not supported");
			pendingTokens.push(Token(TokenType::UNRECOGNIZED, "\\t", currentPos));
			getChar();
			return;
		}

		char next = peekChar();
		if (next == '\n' || next == '#' || next == '\0')
			return;

		int top = spacesStack.top();
		if (spacesCount > top)
		{
			spacesStack.push(spacesCount);
			pendingTokens.push(Token(TokenType::INDENT, "INDENT", start_pos));
		}
		else if (spacesCount < top)
		{
			while (top > spacesCount)
			{
				spacesStack.pop();
				pendingTokens.push(Token(TokenType::DEDENT, "DEDENT", start_pos));

				top = spacesStack.top();
			}

			if (spacesStack.top() != spacesCount)
			{
				logError(currentPos, "INDENT", "Invalid indentation level");
				pendingTokens.push(Token(TokenType::UNRECOGNIZED, "INVALID INDENT", currentPos));
			}
		}
	}

	void processDedentation()
	{	
		auto top = spacesStack.top();
		while (top > 0)
		{
			spacesStack.pop();


			currentPos.col = 1;
			pendingTokens.push(Token(TokenType::DEDENT, "DEDENT", currentPos));
			currentPos.row++;

			top = spacesStack.top();
		}
	}

	char getChar()
	{
		if (isAtEnd(buffPos))
			return '\0';

		currentPos.col++;
		return buffer[buffPos++];
	}

	char peekChar()
	{
		if (isAtEnd(buffPos))
			return '\0';

		return buffer[buffPos];
	}

	bool isAtEnd(int inPos)
	{
		return inPos >= buffer.size();
	}

	bool isAlphaNumeric(char inC)
	{ 
		return (inC >= 'a' && inC <= 'z') || (inC >= 'A' && inC <= 'Z') || inC == '_' || (inC >= '0' && inC <= '9');
	}

	bool isAlphaLetter(char inC)
	{
		return (inC >= 'a' && inC <= 'z') || (inC >= 'A' && inC <= 'Z');
	}

	bool isNumber(char inC)
	{
		return (inC >= '0' && inC <= '9');
	}

	bool isValidStringChar(char inC)
	{
		return inC >= 32 && inC <= 126;
	}

	void logError(const Position& inPos, const std::string& type, const std::string& hint)
	{
		errorCounter++;
		std::cerr << "Error at (" << inPos.row << ":" << inPos.col << ") - "
				  << type << " - " << hint << "\n";
	}
};

#endif