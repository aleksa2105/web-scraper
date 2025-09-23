#include "BookAnalyzer.h"
#include <tbb/parallel_for.h>
#include <tbb/parallel_reduce.h>
#include <fstream>
#include <iostream>


AnalysisResults BookAnalyzer::analyze() {

    if (m_books.empty()) {
        return {};
    }

    countFiveStarBooks();
    calcAverageCost();
    sumInStockBooks();
    findLongestTitleBook();
    findMostReviewedBook();

    // save to file
    writeResultsToFile();

    return m_results;
}

void BookAnalyzer::writeResultsToFile() {
    const std::string filename{ "book_analysis_results.txt" };

    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "Error: could not open file " << filename << " for writing.\n";
        return;
    }

    out << "Five-star books: " << m_results.fiveStarBooks << "\n";
    out << "Average price: " << m_results.averagePrice << "\n";
    out << "Books in stock: " << m_results.booksInStock << "\n";

    out << "\nBook with longest title:\n";
    out << "Title: " << m_results.longestTitleBook.title() << "\n";
    out << "Price: " << m_results.longestTitleBook.price() << "\n";
    out << "Stock: " << m_results.longestTitleBook.stockQuantity() << "\n";
    out << "Reviews: " << m_results.longestTitleBook.numReviews() << "\n";
    out << "URL: " << m_results.longestTitleBook.productUrl().str() << "\n";

    out << "\nBook with most reviews:\n";
    out << "Title: " << m_results.mostReviewedBook.title() << "\n";
    out << "Price: " << m_results.mostReviewedBook.price() << "\n";
    out << "Stock: " << m_results.mostReviewedBook.stockQuantity() << "\n";
    out << "Reviews: " << m_results.mostReviewedBook.numReviews() << "\n";
    out << "URL: " << m_results.mostReviewedBook.productUrl().str() << "\n";

    out.close();
    std::cout << "Analysis results saved to " << filename << std::endl;
}

void BookAnalyzer::countFiveStarBooks() {
    std::atomic<int> fiveStarCount{ 0 };

    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, m_books.size()),
        [&](const tbb::blocked_range<size_t>& range) {
            int localCount = 0;
            for (size_t i = range.begin(); i != range.end(); ++i) {
                if (m_books[i].rating() == ProductRating::five_star) {
                    localCount++;
                }
            }
            fiveStarCount += localCount;
        }
    );

    m_results.fiveStarBooks = fiveStarCount.load();
}

void BookAnalyzer::calcAverageCost() {
    auto range = tbb::blocked_range<size_t>(0, m_books.size());
    double initSum{};

    double totalPrice = tbb::parallel_reduce(
        range,
        initSum,
        [&](const tbb::blocked_range<size_t>& r, double localSum) {
            for (size_t i = r.begin(); i != r.end(); ++i) {
                localSum += m_books[i].price();
            }
            return localSum;
        },
        std::plus<>() // collecting results
    );

    m_results.averagePrice = totalPrice / m_books.size();
}

void BookAnalyzer::sumInStockBooks() {
    std::atomic<int> inStockCount{ 0 };

    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, m_books.size()),
        [&](const tbb::blocked_range<size_t>& range) {
            int localCount = 0;
            for (size_t i = range.begin(); i != range.end(); ++i) {
                if (m_books[i].isInStock()) {
                    localCount++;
                }
            }
            inStockCount += localCount;
        }
    );
    m_results.booksInStock = inStockCount.load();
}

void BookAnalyzer::findLongestTitleBook() {
    auto range = tbb::blocked_range<size_t>(0, m_books.size());
    Book initBook{};

    Book longestTitleBook = tbb::parallel_reduce(
        range,
        initBook,
        [&](const tbb::blocked_range<size_t>& r, Book localLongest) -> Book {
            for (size_t i = r.begin(); i != r.end(); ++i) {
                if (m_books[i].title().length() > localLongest.title().length()) {
                    localLongest = m_books[i];
                }
            }
            return localLongest;
        },
        [](const Book& a, const Book& b) -> Book {
            return (a.title().length() > b.title().length()) ? a : b;
        }
    );

    m_results.longestTitleBook = longestTitleBook;
}

void BookAnalyzer::findMostReviewedBook() {
    auto range = tbb::blocked_range<size_t>(0, m_books.size());
    size_t initIndex{};

    size_t mostReviewedIndex = tbb::parallel_reduce(
        range,
        initIndex,
        [&](const tbb::blocked_range<size_t>& r, size_t localIndex) -> size_t {
            for (size_t i = r.begin(); i != r.end(); ++i) {
                if (m_books[i].numReviews() > m_books[localIndex].numReviews()) {
                    localIndex = i;
                }
            }
            return localIndex;
        },
        [&](size_t a, size_t b) -> size_t {
            return (m_books[a].numReviews() > m_books[b].numReviews()) ? a : b;
        }
    );

    m_results.mostReviewedBook = m_books[mostReviewedIndex];
}
