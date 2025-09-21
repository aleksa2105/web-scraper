#pragma once
#include "../url/Url.h"
#include <string>
#include <vector>
#include <chrono>

struct PageContent {
    Url url;
    std::string html;
    bool success{};
    int statusCode{};
    std::string errorMessage;
};

struct FetchConfig {
    int timeoutSeconds{ 30 };
    int maxRetries{ 3 };
    std::chrono::milliseconds retryDelay{ 1000 };
    std::chrono::milliseconds rateLimitDelay{ 200 };
    int maxConcurrency{ 8 };
    std::string userAgent{ "Mozilla/5.0 (compatible; WebScraper/1.0)" };
};

using PageContentList = std::vector<PageContent>;

class PageFetcher {
public:

    explicit PageFetcher(const FetchConfig& config = FetchConfig{});
    ~PageFetcher();

    /**
     * Complete pipeline: catalog URLs -> all book page HTML
     * 1. Fetch catalog pages
     * 2. Extract book URLs from catalogs
     * 3. Fetch all book pages in parallel
     * @param catalogUrls List of catalog page URLs
     * @return List of book page HTML content
     */
    PageContentList fetchAllBookPages(const UrlList& catalogUrls);

    /**
     * Fetches a single page with retry logic
     * @param url URL to fetch
     * @return PageContent with HTML or error info
     */
    PageContent fetchPage(const Url& url);

    /**
     * Fetches multiple pages in parallel
     * @param urls List of URLs to fetch
     * @return List of PageContent objects
     */
    PageContentList fetchPages(const UrlList& urls);

private:

    FetchConfig m_config;
    std::chrono::steady_clock::time_point m_lastRequestTime;

    // libcurl callback for writing response data
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data);

    /**
     * Internal fetch with retry logic
     */
    PageContent fetchWithRetry(const Url& url);

    /**
     * Extract book URLs from multiple catalog pages
     */
    UrlList extractBookUrls(const PageContentList& catalogPages);

    /**
     * Extract book URLs from a single catalog page HTML
     */
    UrlList extractBookUrlsFromPage(const std::string& html, const Url& catalogUrl);

    /**
     * Apply rate limiting - sleep if needed
     */
    void applyRateLimit();
};