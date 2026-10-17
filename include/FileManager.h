#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <filesystem>

namespace file
{
    inline std::string read_file_to_string(const std::string& filepath)
    {
        std::ifstream file_stream(filepath, std::ios::in | std::ios::binary);

        if (!file_stream.is_open())
            throw std::runtime_error("No se pudo abrir el archivo: " + filepath);

        std::ostringstream buffer;
        buffer << file_stream.rdbuf();
        return buffer.str();
    }

    inline std::vector<std::string> read_directory(const std::string& dirpath)
    {
        if (!std::filesystem::exists(dirpath) || !std::filesystem::is_directory(dirpath))
        {
            throw std::runtime_error("El directorio no existe o no es valido: " + dirpath);
        }

        std::vector<std::string> entries;

        for (const auto& entry : std::filesystem::directory_iterator(dirpath))
        {
            entries.push_back(entry.path().string());
        }

        return entries;
    }

    inline std::vector<std::string> split(const std::string& text)
    {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream token_stream(text);

        while (token_stream >> token)
            tokens.push_back(token);

        return tokens;
    }
}

#endif