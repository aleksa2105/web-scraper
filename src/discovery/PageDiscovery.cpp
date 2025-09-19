#include "PageDiscovery.h"
#include <iostream>
#include <curl/curl.h>
#include <regex>

UrlList PageDiscovery::discoverAllPages(const Url& startUrl) {
    UrlList catalogPages;
    Url currentUrl = startUrl;
    int pageCount = 0;

    std::cout << "Starting page discovery from: " << startUrl << std::endl;

    while (!currentUrl.empty()) {
        pageCount++;
        catalogPages.push_back(currentUrl);
        std::cout << "Page " << pageCount << ": " << currentUrl << std::endl;

        // Fetch HTML content from the current page
        std::string html = fetchHtml(currentUrl);
        if (html.empty()) {
            std::cerr << "Failed to fetch HTML for: " << currentUrl << std::endl;
            break;
        }

        // Extract next page URL
        Url nextUrl = extractNextPageUrl(html, currentUrl);

        // Prevent infinite loop if the same URL is returned
        if (nextUrl == currentUrl) {
            std::cout << "No more pages found (same URL returned)" << std::endl;
            break;
        }

        currentUrl = nextUrl;

        // Safety limit to avoid infinite crawling
        if (pageCount >= 100) {
            std::cout << "Reached page limit (100)" << std::endl;
            break;
        }
    }

    std::cout << "Discovery completed. Found " << catalogPages.size() << " catalog pages." << std::endl;
    return catalogPages;
}

size_t PageDiscovery::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data) {
    size_t totalSize = size * nmemb;
    data->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

std::string PageDiscovery::fetchHtml(const Url& url) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize CURL" << std::endl;
        return "";
    }

    std::string htmlContent;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &htmlContent);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);  // follow redirects
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);        // timeout 30s
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (compatible; WebScraper/1.0)");

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
        return "";
    }

    return htmlContent;
}

Url PageDiscovery::extractNextPageUrl(const std::string& htmlContent, const Url& currentUrl) {
    std::smatch match;

    // Define regex patterns to locate "next" button
    static const std::regex pattern1(R"DELIM(<li\s+class="next">\s*<a\s+href="([^"]+)")DELIM", std::regex::icase);
    static const std::regex pattern2(R"DELIM(class="next"[^>]*>\s*<a[^>]+href="([^"]+)")DELIM", std::regex::icase);

    const std::vector<const std::regex*> patterns = { &pattern1, &pattern2 };

    for (const auto* pattern : patterns) {
        if (std::regex_search(htmlContent, match, *pattern)) {
            std::string relativeUrl = match[1].str();
            // Convert relative URL to absolute using the current page as base
            return currentUrl.makeAbsolute(relativeUrl);
        }
    }

    return Url(); // No next page found
}
