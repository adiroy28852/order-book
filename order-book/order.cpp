#include "order.hpp"

#include <cassert>
#include <stdexcept>

Order::Order(OrderId id, Side side, Price price, Quantity quantity)
: id_(id), side_(side), price_(price), original_quantity_(quantity), remaining_quantity_(quantity) {
    if (id == 0) {
        throw std::invalid_argument("OrderId cant be 0 \n");
    }

    if (price <= 0) {
        throw std::invalid_argument("Order Price must be positive \n");
    }

    if (quantity == 0) {
        throw std::invalid_argument("Order Quantity must be non zero \n");
    }
}

OrderId Order::id() const noexcept {
    return id_;
}

Side Order::side() const noexcept {
    return side_;
}

Price Order::price() const noexcept {
    return  price_;
}

Quantity Order::original_quantity() const noexcept {
    return original_quantity_;
}

Quantity Order::remaining_quantity() const noexcept {
    return remaining_quantity_;
}

bool Order::is_filled() const noexcept {
    return remaining_quantity_ == 0;
}

Quantity Order::fill(Quantity quantity) noexcept {
    assert(quantity > 0);
    assert(quantity <= remaining_quantity_);

    remaining_quantity_ -= quantity;
    return  quantity;
}