#ifndef TABLEDRAWER_H
#define TABLEDRAWER_H

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
        std::string html = R"(<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <title>Tabla de Parseo LR</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background-color: #f9f9f9; }
        h2 { color: #333; }
        table { border-collapse: collapse; width: 100%; max-width: 1000px; background: white; box-shadow: 0 1px 3px rgba(0,0,0,0.2); }
        th, td { border: 1px solid #ccc; padding: 8px 12px; text-align: center; }
        th { background-color: #2c3e50; color: white; }
        th.action-hdr { background-color: #2980b9; }
        th.goto-hdr { background-color: #27ae60; }
        tr:nth-child(even) { background-color: #f2f2f2; }
        .state-col { font-weight: bold; background-color: #eaeded; }
        .empty-cell { color: #aaa; }
    </style>
</head>
<body>
    <h2>Tabla de Parseo - Compilador</h2>
    <table>
        <thead>
            <tr>
                <th rowspan="2">Estado</th>
                <th class="action-hdr" colspan=")";

        html += std::to_string(terminals.size()) + R"(">ACTION (Terminales)</th>
                <th class="goto-hdr" colspan=")";

        html += std::to_string(nonTerminals.size()) + R"(">GOTO (No Terminales)</th>
            </tr>
            <tr>
)";

        for (const auto& term : terminals)
            html += "                <th class=\"action-hdr\">" + term + "</th>\n";
        for (const auto& nonTerm : nonTerminals)
            html += "                <th class=\"goto-hdr\">" + nonTerm + "</th>\n";

        html += R"(            </tr>
        </thead>
        <tbody>
)";

        for (const auto& row : rows)
        {
            html += "            <tr>\n";
            html += "                <td class=\"state-col\">" + std::to_string(row.stateId) + "</td>\n";

            // Terminales
            for (const auto& term : terminals)
            {
                auto it = row.actionMap.find(term);
                if (it != row.actionMap.end() && !it->second.empty())
                    html += "                <td>" + it->second + "</td>\n";
                else
                    html += "                <td class=\"empty-cell\">-</td>\n";
            }

            // GOTO
            for (const auto& nonTerm : nonTerminals)
            {
                auto it = row.gotoMap.find(nonTerm);
                if (it != row.gotoMap.end() && !it->second.empty())
                    html += "                <td>" + it->second + "</td>\n";
                else
                    html += "                <td class=\"empty-cell\">-</td>\n";
            }

            html += "            </tr>\n";
        }

        html += R"(        </tbody>
    </table>
</body>
</html>
)";
        return html;
    }

    void saveToFile(
        const std::string& filename,
        const std::vector<std::string>& terminals,
        const std::vector<std::string>& nonTerminals,
        const std::vector<StateRow>& rows)
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
};

#endif