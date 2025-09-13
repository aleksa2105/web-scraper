#include "Url.h"

#include <fstream>
#include <iostream>
#include <string>

std::vector<std::string> Url::loadFromFile() {
    std::vector<std::string> urls;

    std::ifstream file("../src/url/urls.txt");
    if (!file) {
        std::cerr << "Cannot open file specified\n";
        return urls;
    }

    std::string url;
    while (std::getline(file, url)) {
        if (!url.empty())
            urls.push_back(url);
    }

    file.close();
    return urls;
}

std::vector<std::string> Url::loadFromConsole() {
    std::vector<std::string> urls;

    std::cout << "Enter URLs (one per line, empty line to finish):\n";

    std::string url;
    while (std::getline(std::cin, url)) {
        if (url.empty()) {
            break;
        }
        urls.push_back(url);
    }
    return urls;
}
