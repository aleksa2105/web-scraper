#include "PageFetcher.h"
#include <curl/curl.h>
#include <iostream>
#include <regex>
#include <thread>
#include <algorithm>
#include <tbb/parallel_for.h>
#include <tbb/concurrent_vector.h>
#include <tbb/blocked_range.h>
#include <tbb/task_arena.h>

PageFetcher::PageFetcher(const FetchConfig& config)
    : m_config(config), m_lastRequestTime(std::chrono::steady_clock::now()) {
    // Global CURL initialization
    curl_global_init(CURL_GLOBAL_DEFAULT);
    std::cout << "PageFetcher initialized with " << m_config.maxConcurrency
        << " max concurrent connections" << std::endl;
}

PageFetcher::~PageFetcher() {
    curl_global_cleanup();
}

size_t PageFetcher::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data) {
    size_t totalSize = size * nmemb;
    data->append((char*)contents, totalSize);
    return totalSize;
}

void PageFetcher::applyRateLimit() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastRequestTime);

    if (elapsed < m_config.rateLimitDelay) {
        auto sleepTime = m_config.rateLimitDelay - elapsed;
        std::this_thread::sleep_for(sleepTime);
    }

    m_lastRequestTime = std::chrono::steady_clock::now();
}

PageContent PageFetcher::fetchWithRetry(const Url& url) {
    PageContent result{ url };

    for (int attempt = 0; attempt <= m_config.maxRetries; ++attempt) {
        if (attempt > 0) {
            std::cout << "Retry attempt " << attempt << " for: " << url << std::endl;
            std::this_thread::sleep_for(m_config.retryDelay);
        }

        applyRateLimit();

        CURL* curl = curl_easy_init();
        if (!curl) {
            result.errorMessage = "Failed to initialize CURL";
            continue;
        }

        std::string htmlContent;

        // CURL setup
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &htmlContent);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, m_config.timeoutSeconds);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, m_config.userAgent.c_str());
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        CURLcode res = curl_easy_perform(curl);

        // Get HTTP status code
        long statusCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);
        result.statusCode = static_cast<int>(statusCode);

        curl_easy_cleanup(curl);

        if (res == CURLE_OK && statusCode >= 200 && statusCode < 300) {
            result.html = std::move(htmlContent);
            result.success = true;
            return result;
        }
        else {
            result.errorMessage = "HTTP " + std::to_string(statusCode) + " - " + curl_easy_strerror(res);
        }
    }

    std::cerr << "Failed to fetch after " << (m_config.maxRetries + 1)
        << " attempts: " << url << " (" << result.errorMessage << ")" << std::endl;
    return result;
}

PageContent PageFetcher::fetchPage(const Url& url) {
    return fetchWithRetry(url);
}

PageContentList PageFetcher::fetchPages(const UrlList& urls) {
    if (urls.empty()) {
        std::cout << "No URLs to fetch" << std::endl;
        return {};
    }

    std::cout << "Fetching " << urls.size() << " pages with max "
        << m_config.maxConcurrency << " concurrent connections..." << std::endl;

    tbb::concurrent_vector<PageContent> results;

    // Limit concurrency using task_arena
    tbb::task_arena arena(m_config.maxConcurrency);
    arena.execute([&] {
        tbb::parallel_for(
            tbb::blocked_range<size_t>(0, urls.size()),
            [&](const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    PageContent content = fetchWithRetry(urls[i]);
                    results.push_back(std::move(content));

                    // Progress indicator every 10 pages
                    if ((i + 1) % 10 == 0 || i + 1 == urls.size()) {
                        std::cout << "Progress: " << (i + 1) << "/" << urls.size()
                            << " pages fetched" << std::endl;
                    }
                }
            }
        );
        });

    // Convert concurrent_vector to regular vector
    PageContentList finalResults;
    finalResults.reserve(results.size());
    for (const auto& content : results) {
        finalResults.push_back(content);
    }

    // Print statistics
    size_t successCount = 0;
    for (const auto& content : finalResults) {
        if (content.success) successCount++;
    }

    std::cout << "Fetch completed: " << successCount << "/" << finalResults.size()
        << " pages successful ("
        << (finalResults.size() > 0 ? (successCount * 100 / finalResults.size()) : 0)
        << "%)" << std::endl;

    return finalResults;
}

UrlList PageFetcher::extractBookUrlsFromPage(const std::string& html, const Url& catalogUrl) {
    UrlList bookUrls;

    // Pattern for books.toscrape.com - links to books
    // Format: <h3><a href="../../../catalogue/book-title_123/index.html" title="Book Title">Book Title</a></h3>
    const static std::regex bookPattern(R"DELIM(<h3>\s*<a\s+href="([^"]+)"[^>]*>)DELIM");
    std::sregex_iterator iter(html.begin(), html.end(), bookPattern);
    std::sregex_iterator end;

    for (; iter != end; ++iter) {
        std::string relativeUrl = (*iter)[1].str();
        Url absoluteUrl = catalogUrl.makeAbsolute(relativeUrl);
        if (!absoluteUrl.empty()) {
            bookUrls.push_back(absoluteUrl);
        }
    }

    return bookUrls;
}

UrlList PageFetcher::extractBookUrls(const PageContentList& catalogPages) {
    std::cout << "Extracting book URLs from " << catalogPages.size()
        << " catalog pages..." << std::endl;

    tbb::concurrent_vector<Url> allBookUrls;

    // Parallel extraction from catalog pages
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, catalogPages.size()),
        [&](const tbb::blocked_range<size_t>& range) {
            for (size_t i = range.begin(); i != range.end(); ++i) {
                const auto& page = catalogPages[i];
                if (page.success) {
                    auto bookUrls = extractBookUrlsFromPage(page.html, page.url);
                    for (const auto& bookUrl : bookUrls) {
                        allBookUrls.push_back(bookUrl);
                    }
                }
            }
        }
    );

    // Convert to regular vector and remove duplicates
    UrlList finalUrls;
    finalUrls.reserve(allBookUrls.size());

    for (const auto& url : allBookUrls) {
        // Simple duplicate check
        if (std::find(finalUrls.begin(), finalUrls.end(), url.str()) == finalUrls.end()) {
            finalUrls.push_back(url);
        }
    }

    std::cout << "Extracted " << finalUrls.size() << " unique book URLs" << std::endl;
    return finalUrls;
}

PageContentList PageFetcher::fetchAllBookPages(const UrlList& catalogUrls) {
    if (catalogUrls.empty()) {
        std::cerr << "No catalog URLs provided!" << std::endl;
        return {};
    }

    std::cout << "\n=== PHASE 1: Fetching catalog pages ===" << std::endl;

    // Step 1: Fetch catalog pages
    PageContentList catalogPages = fetchPages(catalogUrls);

    if (catalogPages.empty()) {
        std::cerr << "No catalog pages fetched!" << std::endl;
        return {};
    }

    std::cout << "\n=== PHASE 2: Extracting book URLs ===" << std::endl;

    // Step 2: Extract book URLs from catalogs
    UrlList bookUrls = extractBookUrls(catalogPages);

    if (bookUrls.empty()) {
        std::cerr << "No book URLs found in catalog pages!" << std::endl;
        return {};
    }

    std::cout << "\n=== PHASE 3: Fetching book pages ===" << std::endl;

    // Step 3: Fetch all book pages in parallel
    return fetchPages(bookUrls);
}