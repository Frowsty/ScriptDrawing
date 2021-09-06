#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iostream>

class ScriptHandler
{
private:
    std::vector<std::string> keywords;
    std::pair<std::string, std::string> syntax_results;
public:

    std::pair<std::string, std::string > check_syntax(std::string& keyword);

    std::vector<int> extract_data(std::string& data);

    void add_keywords(std::vector<std::string> keywords);
};