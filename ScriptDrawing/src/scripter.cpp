#include "headers/scripter.h"

void ScriptHandler::add_keywords(std::vector<std::string> keywords)
{
    this->keywords = keywords;
}

std::pair<std::string ,std::string> ScriptHandler::check_syntax(std::string& keyword)
{
    auto return_message = std::make_pair(std::string(), std::string());

    bool found_cmd = false;
    std::string command_found;
    for (auto& command : keywords)
    {
        auto found = keyword.find(command);
        if (found == 0 && found != std::string::npos)
        {
            command_found = keyword.substr(found, command.size());
            found_cmd = true;
        }
    }

    if (found_cmd)
    {
        if (return_message.first != "error")
        {
            if (keyword[command_found.size()] != '(')
            {
                return_message.first = "error";
                return_message.second = "Missing opening '(' before parameters";
            }
            else if (keyword.back() != ')')
            {
                return_message.first = "error";
                return_message.second = "Missing closing ')' after parameters";
            }

            if (return_message.first != "error")
            {
                return_message.first = command_found;
                auto parameters_size = keyword.size() - (command_found.size() + 1);
                return_message.second = keyword.substr(command_found.size() + 1, parameters_size - 1);
            }
        }
    }
    else
    {
        return_message.first = "error";
        return_message.second = "Trying to execute an unknown function";
    }
    syntax_results = return_message;
    return return_message;
}

std::vector<int> ScriptHandler::extract_data(std::string& keyword)
{
    std::vector<int> params;

    std::stringstream ss;

    std::replace(syntax_results.second.begin(), syntax_results.second.end(), ',', ' ');
    std::replace(syntax_results.second.begin(), syntax_results.second.end(), '(', ' ');
    std::replace(syntax_results.second.begin(), syntax_results.second.end(), ')', ' ');

    ss << syntax_results.second;

    std::string temp;
    float found;
    while (!ss.eof()) {
        ss >> temp;
        if (std::stringstream(temp) >> found)
        {
            params.push_back(found);
            temp = "";
        }
    }

    return params;
}