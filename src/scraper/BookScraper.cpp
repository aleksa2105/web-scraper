#include "BookScraper.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <tbb/parallel_invoke.h>

// Static regex patterns for books.toscrape.com
const std::regex BookScraper::s_titlePattern(R"(<h1>([^<]+)</h1>)");
const std::regex BookScraper::s_pricePattern(R"(£([0-9]+\.[0-9]{2}))");
const std::regex BookScraper::s_ratingPattern(R"(star-rating\s+(\w+))");
const std::regex BookScraper::s_stockPattern(R"(In stock \((\d+) available\)|Out of stock)");
const std::regex BookScraper::s_taxPattern(R"(<th>Tax</th>\s*<td>£([0-9]+\.[0-9]{2})</td>)");
const std::regex BookScraper::s_reviewsPattern(R"((\d+)\s+review)");


Book BookScraper::scrapeBookFromHtml(const std::string& html, const Url& bookUrl) {
    std::string title;
    double price{ 0.0 };
    StockStatus stockStatus{ StockStatus::unknown };
    ProductRating rating{ ProductRating::unknown };
    double tax{ 0.0 };
    int stockQuantity{ 0 };
    int numReviews{ 0 };

    tbb::parallel_invoke(
        [&] { title = extractTitle(html); },
        [&] { price = extractPrice(html); },
        [&] { stockStatus = extractStockStatus(html); },
        [&] { rating = extractRating(html); },
        [&] { tax = extractTax(html); },
        [&] { stockQuantity = extractStockQuantity(html); },
        [&] { numReviews = extractNumReviews(html); }
    );

    Book book(title, price, stockStatus,
        rating, tax, stockQuantity, numReviews, bookUrl);

    return book;
}

std::string BookScraper::extractTitle(const std::string& html) {
    std::smatch match;
    if (std::regex_search(html, match, s_titlePattern)) {
        return cleanText(match[1].str());
    }
    return "";
}

double BookScraper::extractPrice(const std::string& html) {
    std::smatch match;
    if (std::regex_search(html, match, s_pricePattern)) {
        return parsePrice(match[1].str());
    }
    return 0.0;
}

ProductRating BookScraper::extractRating(const std::string& html) {
    std::smatch match;
    if (std::regex_search(html, match, s_ratingPattern)) {
        return parseRatingFromClass(match[1].str());
    }
    return ProductRating::unknown;
}

StockStatus BookScraper::extractStockStatus(const std::string& html) {
    if (html.find("In stock") != std::string::npos) {
        return StockStatus::in_stock;
    }
    else if (html.find("Out of stock") != std::string::npos) {
        return StockStatus::out_of_stock;
    }
    return StockStatus::unknown;
}

int BookScraper::extractStockQuantity(const std::string& html) {
    std::smatch match;
    if (std::regex_search(html, match, s_stockPattern)) {
        if (match.size() > 1 && !match[1].str().empty()) {
            return parseStockQuantity(match[1].str());
        }
    }
    return 0;
}

double BookScraper::extractTax(const std::string& html) {
    std::smatch match;
    if (std::regex_search(html, match, s_taxPattern)) {
        return parseTax(match[1].str());
    }
    return 0.0;
}

int BookScraper::extractNumReviews(const std::string& html) {
    std::smatch match;
    if (std::regex_search(html, match, s_reviewsPattern)) {
        try {
            return std::stoi(match[1].str());
        }
        catch (const std::exception&) {
            return 0;
        }
    }
    return 0; // Default 0 if no reviews
}

std::string BookScraper::cleanText(const std::string& text) {
    std::string cleaned;
    cleaned.reserve(text.size());

    bool inSpace = false;

    auto begin = std::find_if_not(text.begin(), text.end(), ::isspace);
    auto end = std::find_if_not(text.rbegin(), text.rend(), ::isspace).base();

    for (auto it = begin; it != end; ++it) {
        unsigned char ch = static_cast<unsigned char>(*it);
        if (std::isspace(ch)) {
            if (!inSpace) {
                cleaned.push_back(' ');
                inSpace = true;
            }
        }
        else {
            cleaned.push_back(ch);
            inSpace = false;
        }
    }

    return cleaned;
}

double BookScraper::parsePrice(const std::string& priceText) {
    try {
        return std::stod(priceText);
    }
    catch (const std::exception&) {
        return 0.0;
    }
}

ProductRating BookScraper::parseRatingFromClass(const std::string& ratingClass) {
    std::string lowerClass = ratingClass;
    std::transform(lowerClass.begin(), lowerClass.end(), lowerClass.begin(), ::tolower);

    if (lowerClass == "one") return ProductRating::one_star;
    if (lowerClass == "two") return ProductRating::two_star;
    if (lowerClass == "three") return ProductRating::three_star;
    if (lowerClass == "four") return ProductRating::four_star;
    if (lowerClass == "five") return ProductRating::five_star;

    return ProductRating::unknown;
}

int BookScraper::parseStockQuantity(const std::string& stockText) {
    try {
        return std::stoi(stockText);
    }
    catch (const std::exception&) {
        return 0;
    }
}

double BookScraper::parseTax(const std::string& taxText) {
    try {
        return std::stod(taxText);
    }
    catch (const std::exception&) {
        return 0.0; // Fallback to 0.0 if parsing fails
    }
}

bool BookScraper::isValidBookPage(const std::string& html) {
    // Check for characteristic book page elements
    return html.find("<h1>") != std::string::npos &&         // Title
        html.find("star-rating") != std::string::npos &&     // Rating
        html.find("£") != std::string::npos &&               // Price
        html.find("availability") != std::string::npos;      // Stock info
}