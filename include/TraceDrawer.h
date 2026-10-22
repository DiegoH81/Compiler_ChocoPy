#ifndef TRACEDRAWER_H
#define TRACEDRAWER_H

#include <inja/inja.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>

struct TraceRow
{
    std::string states;
    std::string symbols;
    std::string input;
    std::string action;
};

class TraceDrawer
{
public:
    TraceDrawer(std::filesystem::path inBaseDir) :
        baseDir(inBaseDir)
    {}

    std::string generateHTML(const std::vector<TraceRow>& rows)
    {
        nlohmann::json data;
        data["rows"] = nlohmann::json::array();

        const std::string prefix = "PANIC_MODE: ";
        int index = 1;
        for (const auto& row : rows)
        {
            nlohmann::json r;
            r["index"] = index++;
            r["states"] = escapeHtml(row.states);
            r["symbols"] = escapeHtml(row.symbols);
            r["input"] = escapeHtml(row.input);

            bool isError = (row.action.rfind(prefix, 0) == 0);
            std::string cleanAction = isError ? row.action.substr(prefix.length()) : row.action;

            r["action"] = escapeHtml(cleanAction);
            r["isError"] = isError;

            data["rows"].push_back(r);
        }

        inja::Environment env;
        std::string templatePath = std::string(CHOCOPY_ROOT) + "/templates/trace_template.html";
        return env.render_file(templatePath, data);
    }

    void saveToFile(const std::string& filename, const std::vector<TraceRow>& rows)
    {
        std::filesystem::path tracesDir = baseDir / "traces";

        if (!std::filesystem::exists(tracesDir))
            std::filesystem::create_directories(tracesDir);

        std::filesystem::path finalPath = tracesDir / filename;
        std::ofstream file(finalPath.string());

        if (!file.is_open())
        {
            std::cerr << "ERROR: Could not save HTML file in: " << finalPath << "\n";
            return;
        }

        file << generateHTML(rows);
        file.close();

        std::cout << "Trace saved at: " << finalPath << "\n";
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