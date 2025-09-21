#pragma once

#include "Product.h"
#include <string>
#include <vector>

class Book : public Product {
private:

public:
    // Constructors
    Book() = default;

    Book(const std::string& title,
        double price, StockStatus stockStatus, const ProductRating& rating,
        const std::string& description, const std::string& upc,
        double tax, int stockQuantity, int numReviews,
        const Url& imageUrl, const Url& productUrl)
        : Product(title, price, stockStatus, rating, description,
            upc, ProductType::book, tax, stockQuantity, numReviews,
            imageUrl, productUrl) {
    }

    // Utility Methods
    std::string str() const override;
    bool isValid() const { return !m_title.empty() && m_price > 0.0; }
};

using BookList = std::vector<Book>;