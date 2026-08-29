#include "book.hpp"

#include <stdexcept>
#include <utility>

Order* Book::find_order(OrderId id) noexcept {
    const auto it = orders_.find(id);

    if (it == orders_.end()) return nullptr;

    return it->second.get();
}

const Order* Book::find_order(OrderId id) const noexcept {
    const auto it = orders_.find(id);

    if (it == orders_.end()) return nullptr;

    return it->second.get();
}


Order& Book::add_limit_order(OrderId id, Side side, Price price, Quantity quantity) {
    if (orders_.contains(id)) {
        throw std::invalid_argument("Order ID already exists\n");
    }

    auto order = std::make_unique<Order>(id, side, price, quantity);
    Order& reference = *order;
    // first validate get/create limit, create order, then emplace into orders_. 
    PriceLevels& lvls = side == Side::Buy ? bids_ : asks_;

    Limit& limit = lvls.get_or_create(price);

    orders_.emplace(id, std::move(order));
    limit.add_order(reference);

    return reference;
}

const PriceLevels& Book::bids() const noexcept {
    return bids_;
}

const PriceLevels& Book::asks() const noexcept {
    return asks_;
}