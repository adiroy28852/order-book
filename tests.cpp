#include "order-book/book.hpp"

#include <cassert>
#include <iostream>
#include <stdexcept>

int main() {
    Book book;

    auto& order1 = book.add_limit_order(1, Side::Buy, 100, 50);
    auto& order2 = book.add_limit_order(2, Side::Buy, 100, 30);
    auto& order3 = book.add_limit_order(3, Side::Buy, 105, 20);

    assert(order1.id() == 1);
    assert(order2.id() == 2);
    assert(order3.id() == 3);

    assert(book.find_order(1) != nullptr);
    assert(book.find_order(2) != nullptr);
    assert(book.find_order(999) == nullptr);

    assert(book.bids().size() == 2);
    assert(book.bids().best().price() == 105);
    assert(book.bids().best().total_quantity() == 20);

    assert(book.bids().find(100) != nullptr);
    assert(book.bids().find(100)->order_count() == 2);
    assert(book.bids().find(100)->total_quantity() == 80);
    assert(book.bids().find(100)->front().id() == 1);

    auto& ask1 = book.add_limit_order(4, Side::Sell, 110, 40);
    auto& ask2 = book.add_limit_order(5, Side::Sell, 105, 20);
    auto& ask3 = book.add_limit_order(6, Side::Sell, 110, 10);

    assert(ask1.id() == 4);
    assert(ask2.id() == 5);
    assert(ask3.id() == 6);

    assert(book.asks().size() == 2);
    assert(book.asks().best().price() == 105);
    assert(book.asks().best().total_quantity() == 20);

    assert(book.asks().find(110) != nullptr);
    assert(book.asks().find(110)->order_count() == 2);
    assert(book.asks().find(110)->total_quantity() == 50);

    bool threw = false;

    try {
        book.add_limit_order(1, Side::Buy, 200, 10);
    } catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);

    for (OrderId id = 1000; id < 10000; ++id) {
        book.add_limit_order(id, Side::Buy, 200, 1);
    }

    assert(book.find_order(1) != nullptr);
    assert(book.find_order(1)->id() == 1);
    assert(book.find_order(2) != nullptr);
    assert(book.find_order(2)->id() == 2);

    assert(book.bids().find(100) != nullptr);
    assert(book.bids().find(100)->front().id() == 1);

    std::cout << "All tests passed\n";
}