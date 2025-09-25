#pragma once
#include <string>
#include <vector>
#include "../url/Url.h"

enum class StockStatus {
    in_stock,
    out_of_stock,
    unknown
};

enum class ProductRating {
    one_star = 1,
    two_star,
    three_star,
    four_star,
    five_star,
    unknown = 0
};

class Book {
private:

    // Base Product Information
    std::string m_title;
    double m_price{};
    StockStatus m_stockStatus{ StockStatus::unknown };
    ProductRating m_rating{ ProductRating::unknown };

    // Product Information
    double m_tax{};
    int m_stockQuantity{};
    int m_numReviews{};

    // Links
    Url m_productUrl;

public:
    // Constructors
    Book() = default;

    Book(const std::string& title, double price, StockStatus stockStatus, const ProductRating& rating,
        double tax, int stockQuantity, int numReviews, const Url& productUrl)
        : m_title(title), m_price(price), m_stockStatus(stockStatus), m_rating(rating)
        , m_tax(tax), m_stockQuantity(stockQuantity), m_numReviews(numReviews)
        , m_productUrl(productUrl) {
    }

    // Utility Methods
    std::string str() const;
    bool isValid() const { return !m_title.empty() && m_price > 0.0; }

    // Access
    const std::string& title() const { return m_title; }
    double price() const { return m_price; }
    StockStatus stockStatus() const { return m_stockStatus; }
    const ProductRating& rating() const { return m_rating; }
    double tax() const { return m_tax; }
    int stockQuantity() const { return m_stockQuantity; }
    int numReviews() const { return m_numReviews; }
    const Url& productUrl() const { return m_productUrl; }

    // Utility methods
    double priceWithTax() const { return m_price + m_tax; }
    bool isInStock() const { return m_stockStatus == StockStatus::in_stock && m_stockQuantity > 0; }
};

using BookList = std::vector<Book>;

std::string stockStatusStr(StockStatus status);
std::string productRatingStr(ProductRating rating);
