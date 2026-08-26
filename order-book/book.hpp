#pragma once

#include "order.hpp"
#include "pricelevels.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <unordered_map>

class Book {
private:
    std::unordered_map<OrderId, std::unique_ptr<Order>> orders_;
    PriceLevels bids_{PriceOrder::Descending};
    PriceLevels asks_{PriceOrder::Ascending};
public:
    Book() = default;
    ~Book() = default;

    Book(const Book&) = delete;
    Book& operator=(const Book&) = delete;

    Book(Book&&) noexcept = default;
    Book& operator=(Book&&) noexcept = default;

    Order* find_order(OrderId id) noexcept;
    const Order* find_order(OrderId id) const noexcept;

    Order& add_order(OrderId id, Side side, Price price, Quantity quantity);
    Order& add_limit_order(OrderId id, Side side, Price price, Quantity quantity);
};