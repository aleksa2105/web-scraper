#pragma once
#include <string>
#include "../products/Book.h"
#include <tbb/concurrent_vector.h>

struct AnalysisResults {
    int fiveStarBooks{};
    double averagePrice{};
    int booksInStock{};
    Book longestTitleBook;
    Book mostReviewedBook;
};

class BookAnalyzer {
public:

    BookAnalyzer(tbb::concurrent_vector<Book>& books)
        : m_books{ books } {
    }

    AnalysisResults analyze();

    const AnalysisResults& results() const { return m_results; }

private:

    void countFiveStarBooks();

    void calcAverageCost();

    void sumInStockBooks();

    void findLongestTitleBook();

    void findMostReviewedBook();

    void writeResultsToFile();

private:

    tbb::concurrent_vector<Book> m_books;
    AnalysisResults m_results;

};