#pragma once

#include <string>
#include <vector>

class Url {
private:
    std::string m_url;

public:
    Url(const std::string& url) : m_url(url) {}

    const std::string& str() const { return m_url; }

    bool empty() const { return m_url.empty(); }

    bool isAbsolute() const { return m_url.find("http") == 0; }

    bool operator==(const Url& other) const { return m_url == other.m_url; }

    friend std::ostream& operator<<(std::ostream& out, const Url& url) {
        return out << url.m_url;
    }
};

using UrlList = std::vector<Url>;