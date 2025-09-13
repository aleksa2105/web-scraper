#pragma once

#include <string>
#include <vector>

class Url {
public:
    static std::vector<std::string> loadFromFile();
    static std::vector<std::string> loadFromConsole();
};
