#include "order-book/book.hpp"

#include <iostream>

int main() {
    Book book;
    book.add_limit_order(1, Side::Sell, 100, 50);
    book.add_limit_order(2, Side::Sell, 105, 30);

    const auto trades = book.execute_limit_order(3, Side::Buy, 100, 20);
    for (const Trade& trade : trades) {
        std::cout << "Trade: " << trade.quantity << " @ " << trade.price
                  << " (maker " << trade.maker_order_id << ", taker "
                  << trade.taker_order_id << ")\n";
    }
}
