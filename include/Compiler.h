#ifndef COMPILER_H
#define COMPILER_H

#include <fstream>
#include <string>
#include <sstream>

class Compiler
{
public:

private:
	std::string buffer;

	void loadFile(const std::string& source)
	{
		std::ifstream file(source);
		std::stringstream bufferSS;
		bufferSS << file.rdbuf();

		buffer = bufferSS.str();

		file.close();
	}

};

#endif