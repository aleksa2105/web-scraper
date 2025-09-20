#include "Book.h"

std::string Book::str() const {
    return "Book{title='" + m_title + "', price=" + std::to_string(m_price) +
        ", rating=" + productRatingStr(m_rating) + ", stock=" + stockStatusStr(m_stockStatus) + "}";
}
