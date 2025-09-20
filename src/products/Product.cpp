#include "Product.h"

std::string Product::str() const {
    return "Product{title='" + m_title + "', price=" + std::to_string(m_price) +
        ", rating=" + productRatingStr(m_rating) + ", stock=" + stockStatusStr(m_stockStatus) + "}";
}

// Enum to_string implementations
std::string stockStatusStr(StockStatus status) {
    switch (status) {
    case StockStatus::in_stock: return "in stock";
    case StockStatus::out_of_stock: return "out of stock";
    default: return "unknown";
    }
}

std::string productRatingStr(ProductRating rating) {
    switch (rating) {
    case ProductRating::one_star: return "⋆";
    case ProductRating::two_star: return "⋆⋆";
    case ProductRating::three_star: return "⋆⋆⋆";
    case ProductRating::four_star: return "⋆⋆⋆⋆";
    case ProductRating::five_star: return "⋆⋆⋆⋆⋆";
    default: return "unknown";
    }
}

std::string productTypeStr(ProductType type) {
    switch (type) {
    case ProductType::book: return "book";
    default: return "unknown";
    }
}