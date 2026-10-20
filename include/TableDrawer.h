#ifndef TABLEDRAWER_H
#define TABLEDRAWER_H

#include <inja/inja.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <filesystem>

class StateRow
{
public:
    int stateId;
    std::map<std::string, std::string> actionMap;
    std::map<std::string, std::string> gotoMap;

    StateRow(int inId, std::map<std::string, std::string> inActionMap, std::map<std::string, std::string> inGotoMap) :
        stateId(inId), actionMap(inActionMap), gotoMap(inGotoMap)
    {}
};

class TableDrawer
{
public:
    TableDrawer(std::filesystem::path inBaseDir) :
        baseDir(inBaseDir)
    {}

    std::string generateHTML(const std::vector<std::string>& terminals,
        const std::vector<std::string>& nonTerminals,
        const std::vector<StateRow>& rows)
    {
        nlohmann::json data;

        data["terminals"] = nlohmann::json::array();
        for (const auto& t : terminals)
            data["terminals"].push_back(escapeHtml(t));

        data["nonTerminals"] = nlohmann::json::array();
        for (const auto& n : nonTerminals)
            data["nonTerminals"].push_back(escapeHtml(n));

        data["rows"] = nlohmann::json::array();

        for (const auto& row : rows)
        {
            nlohmann::json r;
            r["id"] = row.stateId;
            r["cells"] = nlohmann::json::array();

            for (const auto& term : terminals)
            {
                auto it = row.actionMap.find(term);
                r["cells"].push_back(
                    (it != row.actionMap.end() && !it->second.empty()) ? escapeHtml(it->second) : "-");
            }

            for (const auto& nonTerm : nonTerminals)
            {
                auto it = row.gotoMap.find(nonTerm);
                r["cells"].push_back(
                    (it != row.gotoMap.end() && !it->second.empty()) ? escapeHtml(it->second) : "-");
            }

            data["rows"].push_back(r);
        }

        inja::Environment env;

        std::string templatePath = std::string(CHOCOPY_ROOT) + "/templates/table_template.html";
        return env.render_file(templatePath, data);
    }

    void saveToFile( const std::string& filename, 
                     const std::vector<std::string>& terminals, 
                     const std::vector<std::string>& nonTerminals, const std::vector<StateRow>& rows)
    {
        std::filesystem::path tablesDir = baseDir / "tables";

        if (!std::filesystem::exists(tablesDir))
            std::filesystem::create_directories(tablesDir);

        std::filesystem::path finalPath = tablesDir / filename;
        std::ofstream file(finalPath.string());

        if (!file.is_open())
        {
            std::cerr << "ERROR: Could not save HTML file in: " << finalPath << "\n";
            return;
        }

        file << generateHTML(terminals, nonTerminals, rows);
        file.close();

        std::cout << "Tabla saved at: " << finalPath << "\n";
    }

private:
    std::filesystem::path baseDir;

    static std::string escapeHtml(const std::string& s)
    {
        std::string out;
        out.reserve(s.size());
        for (char c : s)
        {
            switch (c)
            {
            case '&':
                out += "&amp;";
                break;
            case '<':
                out += "&lt;";
                break;
            case '>':
                out += "&gt;";
                break;
            case '"':
                out += "&quot;";
                break;
            case '\'':
                out += "&#39;";
                break;
            default:
                out += c;
                break;
            }
        }
        return out;
    }
};

#endif