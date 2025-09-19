#pragma once
#include <string>
#include <vector>
#include <iostream>

class Url {
private:

    std::string m_url;

public:

    // Constructors
    Url() = default;
    Url(const std::string& url) : m_url(url) {}

    // URL manipulation
    static Url makeAbsoluteUrl(const std::string& relativeUrl, const Url& baseUrl);

    Url makeAbsolute(const std::string& relativeUrl) const;

    Url getBaseDomain() const;

    Url getBasePath() const;

    // Access
    const std::string& str() const { return m_url; }
    const char* c_str() const { return m_url.c_str(); }

    // Checks
    bool empty() const { return m_url.empty(); }
    bool isAbsolute() const { return m_url.find("://") != std::string::npos; }
    bool isRelative() const { return !isAbsolute() && !empty(); }

    // Operators
    bool operator==(const Url& other) const { return m_url == other.m_url; }
    bool operator!=(const Url& other) const { return m_url != other.m_url; }
    friend std::ostream& operator<<(std::ostream& os, const Url& url);

};

using UrlList = std::vector<Url>;