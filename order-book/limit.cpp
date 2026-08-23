#include "limit.hpp"
#include "order.hpp"

#include <cassert>
#include <stdexcept>
#include <utility>

Limit::Limit(Price price) : price_(price) {
    if (price <= 0) {
        throw std::invalid_argument("Limit price must be positive \n");
    }
}

Price Limit::price() const noexcept {
    return price_;
}

Quantity Limit::total_quantity() const noexcept {
    return total_quantity_;
}

std::size_t Limit::order_count() const noexcept {
    return orders_.size();
}

bool Limit::empty() const noexcept {
    return orders_.empty();
}

void Limit::add_order(Order order) {
    if (order.price() != price_) {
        throw std::invalid_argument("Order price != limit price \n");
    }

    if (order.is_filled()) {
        throw std::invalid_argument("Cannot add a filled order to a limit \n");
    }

    total_quantity_ += order.remaining_quantity();
    orders_.push_back(std::move(order));
}

Order& Limit::front() noexcept {
    assert(!orders_.empty());

    return orders_.front();
}

const Order& Limit::front() const noexcept {
    assert(!orders_.empty());

    return orders_.front();
}

Quantity Limit::execute(Quantity quantity) {
    if (quantity == 0) return 0;

    Quantity executed {0};

    while (quantity > 0 && !orders_.empty()) {
        Order& order = orders_.front();

        const Quantity available = order.remaining_quantity();
        const Quantity fill_quantity = available < quantity ? available : quantity;

        order.fill(fill_quantity);
        quantity -= fill_quantity;
        executed += fill_quantity;
        total_quantity_ -= fill_quantity;

        if(order.is_filled()) {
            orders_.pop_front();
        }
    }
    return executed;
}

void Limit::remove_front() noexcept {
    assert(!orders_.empty());

    total_quantity_ -= orders_.front().remaining_quantity();
    orders_.pop_front();
}