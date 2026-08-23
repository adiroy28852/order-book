#include "book.hpp"
#include "limit.hpp"
#include "order.hpp"
#include "pricelevels.hpp"

#include <cstddef>
#include <stdexcept>
#include <utility>

Order* Book::find_order(OrderId id) noexcept {
    const auto it = orders_.find(id);

    if (it == end(orders_)) return NULL;

    return it->second.get();
}

const Order* Book::find_order(OrderId id) const noexcept {
    const auto it = orders_.find(id);

    if (it == end(orders_)) return NULL;

    return it->second.get();
}

Order& Book::add_order(OrderId id, Side side, Price price, Quantity quantity) {
    if (orders_.contains(id)) {
        throw  std::invalid_argument("Order id already exists \n");
    }

    auto order = std::make_unique<Order> (id, side, price, quantity);
    Order& reference = *order;
    
    orders_.emplace(id, std::move(order));

    return reference;
}

Order& Book::add_limit_order(OrderId id, Side side, Price price, Quantity quantity) {
    if (orders_.contains(id)) {
        throw std::invalid_argument("Order ID already exists\n");
    }

    auto order = std::make_unique<Order>(id, side, price, quantity);
    Order& reference = *order;

    orders_.emplace(id, std::move(order));

    PriceLevels& levels = side == Side::Buy ? bids_ : asks_;
    Limit& limit = levels.get_or_create(price);

    limit.add_order(reference);
    return reference;
}