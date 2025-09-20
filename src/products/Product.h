#pragma once
#include <string>
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

enum class ProductType {
    book,
    unknown
};

class Product {
protected:

    // Base Product Information
    std::string m_title;
    double m_price{};
    StockStatus m_stockStatus{ StockStatus::unknown };
    ProductRating m_rating{ ProductRating::unknown };

    // Product Description
    std::string m_description;

    // Product Information
    std::string m_upc;
    ProductType m_type{ ProductType::unknown };
    double m_tax{};
    int m_stockQuantity{};
    int m_numReviews{};

    // Links
    Url m_imageUrl;
    Url m_productUrl;

public:

    // Constructors & Destructors
    Product() = default;

    Product(const std::string& title,
        double price, StockStatus stockStatus, const ProductRating& rating,
        const std::string& description, const std::string& upc, ProductType type,
        double tax, int stockQuantity, int numReviews,
        const Url& imageUrl, const Url& productUrl)
        : m_title(title), m_price(price), m_stockStatus(stockStatus)
        , m_rating(rating), m_description(description), m_upc(upc)
        , m_type(type), m_tax(tax), m_stockQuantity(stockQuantity)
        , m_numReviews(numReviews), m_imageUrl(imageUrl), m_productUrl(productUrl) {
    }

    virtual ~Product() = default;

    // Access
    const std::string& title() const { return m_title; }
    double price() const { return m_price; }
    StockStatus stockStatus() const { return m_stockStatus; }
    const ProductRating& rating() const { return m_rating; }
    const std::string& description() const { return m_description; }
    const std::string& upc() const { return m_upc; }
    ProductType type() const { return m_type; }
    double tax() const { return m_tax; }
    int stockQuantity() const { return m_stockQuantity; }
    int numReviews() const { return m_numReviews; }
    const Url& imageUrl() const { return m_imageUrl; }
    const Url& productUrl() const { return m_productUrl; }

    // Utility methods
    double priceWithTax() const { return m_price + m_tax; }
    bool isInStock() const { return m_stockStatus == StockStatus::in_stock && m_stockQuantity > 0; }
    virtual std::string str() const;

};

std::string stockStatusStr(StockStatus status);
std::string productRatingStr(ProductRating rating);
std::string productTypeStr(ProductType type);
