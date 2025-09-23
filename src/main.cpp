#include <iostream>
#include <fstream>
#include <atomic>
#include <algorithm>
#include <numeric>
#include <unordered_map>
#include <tbb/parallel_pipeline.h>
#include <tbb/concurrent_vector.h>
#include <tbb/concurrent_unordered_set.h>
#include <tbb/concurrent_queue.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_reduce.h>
#include <thread>
#include "url/Url.h"
#include "discovery/PageDiscovery.h"
#include "fetcher/PageFetcher.h"
#include "products/Book.h"
#include "scraper/BookScraper.h"
#include "analyzer/BookAnalyzer.h"

struct PipeLineResult {
    tbb::concurrent_vector<Book> books;
    tbb::concurrent_unordered_set<Url, UrlHash> visitedUrls;
};

tbb::concurrent_vector<Url> extractBookUrls(
    const UrlList& catalogUrls,
    PageFetcher& fetcher,
    tbb::concurrent_unordered_set<Url, UrlHash>& visitedUrls
) {
    tbb::concurrent_vector<Url> allBookUrls;

    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, catalogUrls.size()),
        [&](const tbb::blocked_range<size_t>& range) {
            for (size_t i = range.begin(); i != range.end(); ++i) {
                const auto& catalogUrl = catalogUrls[i];
                std::cout << "Processing catalog: " << catalogUrl << std::endl;

                PageContent catalogPage = fetcher.fetchPage(catalogUrl);
                if (!catalogPage.success) {
                    std::cerr << "Failed to fetch catalog: " << catalogUrl << std::endl;
                    continue;
                }

                tbb::concurrent_queue<Url> tempQueue;
                fetcher.extractBookUrlsFromPage(tempQueue, catalogPage.html, catalogPage.url);

                Url bookUrl;
                while (tempQueue.try_pop(bookUrl)) {
                    if (visitedUrls.insert(bookUrl).second) {
                        allBookUrls.push_back(bookUrl);
                    }
                }
            }
        }
    );

    return allBookUrls;
}

tbb::concurrent_vector<Book> fetchAndScrapeBooks(
    const tbb::concurrent_vector<Url>& allBookUrls,
    PageFetcher& fetcher,
    BookScraper& scraper,
    int maxConcurrency = 128
) {
    tbb::concurrent_vector<Book> books;
    auto bookIter = allBookUrls.cbegin();

    tbb::parallel_pipeline(
        maxConcurrency,

        // Stage 1: Source book URLs
        tbb::make_filter<void, Url>(
            tbb::filter_mode::serial_out_of_order,
            [&](tbb::flow_control& fc) -> Url {
                if (bookIter == allBookUrls.cend()) {
                    fc.stop();
                    return Url{};
                }
                return *bookIter++;
            }
        ) &

        // Stage 2: Fetch book page
        tbb::make_filter<Url, PageContent>(
            tbb::filter_mode::parallel,
            [&](const Url& bookUrl) -> PageContent {
                std::cout << "Fetching book: " << bookUrl << std::endl;
                return fetcher.fetchPage(bookUrl);
            }
        ) &

        // Stage 3: Scrape book
        tbb::make_filter<PageContent, void>(
            tbb::filter_mode::parallel,
            [&](const PageContent& page) -> void {
                if (page.success) {
                    std::cout << "Scraping book: " << page.url << std::endl;
                    books.emplace_back(scraper.scrapeBookFromHtml(page.html, page.url));
                }
                else {
                    std::cerr << "Failed to fetch book: " << page.url << std::endl;
                }
            }
        )
    );

    return books;
}

PipeLineResult runPipeLine(const UrlList& catalogUrls, PageFetcher& fetcher, BookScraper& scraper) {
    tbb::concurrent_unordered_set<Url, UrlHash> visitedUrls;

    auto allBookUrls = extractBookUrls(catalogUrls, fetcher, visitedUrls);
    std::cout << "Total unique book URLs to process: " << allBookUrls.size() << std::endl;

    auto books = fetchAndScrapeBooks(allBookUrls, fetcher, scraper);

    return { books, visitedUrls };
}


int main() {
    PageFetcher fetcher{};
    BookScraper scraper{};

    UrlList catalogUrls = readCatalogUrlsFromFile();

    auto start = std::chrono::steady_clock::now();

    PipeLineResult pipeLineResult = runPipeLine(catalogUrls, fetcher, scraper);

    auto end = std::chrono::steady_clock::now();
    double totalTime = std::chrono::duration<double>(end - start).count();

    std::cout << "\n=== BASIC RESULTS ===" << std::endl;
    std::cout << "Total books scraped: " << pipeLineResult.books.size() << std::endl;
    std::cout << "Total URLs visited: " << pipeLineResult.visitedUrls.size() << std::endl;
    std::cout << "Processing time: " << totalTime << " seconds" << std::endl;

    if (!pipeLineResult.books.empty()) {
        std::cout << "Throughput: " << (pipeLineResult.books.size() / totalTime) << " books/second" << std::endl;
    }

    BookAnalyzer analyzer{ pipeLineResult.books };
    analyzer.analyze();

    return 0;
}