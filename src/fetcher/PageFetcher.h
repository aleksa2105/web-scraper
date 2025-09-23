#pragma once
#include "../url/Url.h"
#include <string>
#include <vector>
#include <chrono>
#include <tbb/concurrent_queue.h>
#include <thread>

struct PageContent {
    Url url;
    std::string html;
    bool success{};
    int statusCode{};
    std::string errorMessage;
};

struct FetchConfig {
    int timeoutSeconds{ 5 };
    int maxRetries{ 50 };
    std::chrono::milliseconds retryDelay{ 50 };
    std::chrono::milliseconds rateLimitDelay{ 10 };
    unsigned int maxConcurrency{ std::thread::hardware_concurrency() };
    std::string userAgent{ "Mozilla/5.0 (compatible; WebScraper/1.0)" };
};

class PageFetcher {
public:

    explicit PageFetcher(const FetchConfig& config = FetchConfig{});

    /**
     * Fetches a single page with retry logic
     * @param url URL to fetch
     * @return PageContent with HTML or error info
     */
    PageContent fetchPage(const Url& url);

    /**
     * Extract book URLs from a single catalog page HTML
     */
    void extractBookUrlsFromPage(tbb::concurrent_queue<Url>& bookQueue, const std::string& html, const Url& catalogUrl);

    const FetchConfig& config() const { return m_config; }

    ~PageFetcher();

private:

    // libcurl callback for writing response data
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data);

    /**
     * Internal fetch with retry logic
     */
    PageContent fetchWithRetry(const Url& url);

    /**
     * Apply rate limiting
     */
    void applyRateLimit();

private:

    FetchConfig m_config;
    std::chrono::steady_clock::time_point m_lastRequestTime;

};