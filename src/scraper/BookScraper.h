#pragma once

#include "../products/Book.h"
#include "../fetcher/PageFetcher.h"
#include <vector>
#include <string>
#include <regex>

using BookList = std::vector<Book>;

class BookScraper {
public:

    /**
     * Scrapes a single book from its HTML content.
     *
     * @param html HTML content of the book page
     * @param bookUrl URL of the book page
     * @return Book object populated with extracted data
     */
    static Book scrapeBookFromHtml(const std::string& html, const Url& bookUrl);

private:

    // Helper methods for parsing specific parts of HTML

    static std::string extractTitle(const std::string& html);

    static double extractPrice(const std::string& html);

    static ProductRating extractRating(const std::string& html);

    static StockStatus extractStockStatus(const std::string& html);

    static int extractStockQuantity(const std::string& html);

    static std::string extractDescription(const std::string& html);

    static std::string extractUpc(const std::string& html);

    static double extractTax(const std::string& html);

    static int extractNumReviews(const std::string& html);

    static Url extractImageUrl(const std::string& html, const Url& baseUrl);

    // Utility methods

    static std::string cleanText(const std::string& text);

    static double parsePrice(const std::string& priceText);

    static ProductRating parseRatingFromClass(const std::string& ratingClass);

    static int parseStockQuantity(const std::string& stockText);

    static double parseTax(const std::string& taxText);

    static bool isValidBookPage(const std::string& html);

private:

    // Regex patterns for books.toscrape.com
    static const std::regex s_titlePattern;
    static const std::regex s_pricePattern;
    static const std::regex s_ratingPattern;
    static const std::regex s_stockPattern;
    static const std::regex s_descriptionPattern;
    static const std::regex s_upcPattern;
    static const std::regex s_imagePattern;
    static const std::regex s_taxPattern;
    static const std::regex s_reviewsPattern;

};
