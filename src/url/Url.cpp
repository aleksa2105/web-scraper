#include "Url.h"

Url Url::makeAbsolute(const std::string& relativeUrl) const {
    return makeAbsoluteUrl(relativeUrl, *this);
}

Url Url::getBaseDomain() const {
    if (!isAbsolute()) return Url();

    // Find protocol position (e.g., "http://", "https://")
    size_t protocolEnd = m_url.find("://");
    if (protocolEnd == std::string::npos) {
        return Url(); // invalid URL
    }

    size_t hostStart = protocolEnd + 3;

    // Find the first '/' after the host (end of domain)
    size_t domainEnd = m_url.find('/', hostStart);

    if (domainEnd != std::string::npos) {
        return Url(m_url.substr(0, domainEnd));
    }
    return Url(m_url); // whole string is just domain
}

Url Url::getBasePath() const {
    if (!isAbsolute()) return Url();

    // Find protocol position
    size_t protocolEnd = m_url.find("://");
    if (protocolEnd == std::string::npos) {
        return Url();
    }

    size_t hostStart = protocolEnd + 3;
    size_t lastSlash = m_url.find_last_of('/');

    // No slash after host -> add '/'
    if (lastSlash == std::string::npos || lastSlash < hostStart) {
        return Url(m_url + "/");
    }

    // Already ends with '/' -> return as-is
    if (lastSlash == m_url.size() - 1) {
        return Url(m_url);
    }

    // Otherwise, cut at the last '/'
    return Url(m_url.substr(0, lastSlash + 1));
}

Url Url::makeAbsoluteUrl(const std::string& relativeUrl, const Url& baseUrl) {
    if (relativeUrl.empty()) return Url();

    // Already absolute (contains "://")
    if (relativeUrl.find("://") != std::string::npos) {
        return Url(relativeUrl);
    }

    // Protocol-relative URL (e.g., //cdn.example.com)
    if (relativeUrl.size() > 1 && relativeUrl[0] == '/' && relativeUrl[1] == '/') {
        size_t protocolEnd = baseUrl.m_url.find("://");
        if (protocolEnd != std::string::npos) {
            std::string protocol = baseUrl.m_url.substr(0, protocolEnd);
            return Url(protocol + ":" + relativeUrl);
        }
        return Url("http:" + relativeUrl); // fallback
    }

    // No valid baseUrl -> return as-is
    if (baseUrl.empty() || !baseUrl.isAbsolute()) {
        return Url(relativeUrl);
    }

    if (relativeUrl[0] == '/') {
        // Root-relative path -> base domain + path
        return Url(baseUrl.getBaseDomain().str() + relativeUrl);
    }
    else {
        // Relative path -> base path + relative part
        return Url(baseUrl.getBasePath().str() + relativeUrl);
    }
}


std::ostream& operator<<(std::ostream& out, const Url& url) {
    return out << url.m_url;
}