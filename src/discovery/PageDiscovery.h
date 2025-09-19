#pragma once

#include "../url/Url.h"

class PageDiscovery {
public:

    /**
     * Discovers all catalog pages starting from a given URL.
     * Follows "next" page links until no more pages are found.
     *
     * @param startUrl The initial catalog page URL
     * @return List of all discovered catalog page URLs
     */
    UrlList discoverAllPages(const Url& startUrl);

private:

    /**
     * Callback function for libcurl to write response data
     */
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data);

    /**
     * Fetches HTML content from the given URL using libcurl
     *
     * @return HTML content as string
     */
    std::string fetchHtml(const Url& url);

    /**
     * Extracts the "next page" URL from HTML content
     *
     * @param htmlContent HTML content to parse
     * @param currentUrl Current page URL for making absolute URLs
     * @return Next page URL, empty string if not found
     */
    Url extractNextPageUrl(const std::string& htmlContent, const Url& currentUrl);

};