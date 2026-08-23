#pragma once

#include "order.hpp"
#include <cstddef>
#include <list>

class Limit {
public: 
    explicit Limit(Price price);
    Limit(const Limit&) = delete;
    Limit& operator=(const Limit&) = delete;

    Limit(Limit&&) noexcept = default;
    Limit& operator=(Limit&&) noexcept = default;

    ~Limit() = default;

    [[nodiscard]] Price price() const noexcept;
    [[nodiscard]] Quantity total_quantity() const noexcept;
    [[nodiscard]] std::size_t order_count() const noexcept;
    [[nodiscard]] bool empty() const noexcept;

    void add_order(Order order);
    
    [[nodiscard]] Order& front() noexcept;
    [[nodiscard]] const Order& front() const noexcept;

    Quantity execute(Quantity quantity);
    
    void remove_front() noexcept;
private:
    Price price_;
    Quantity total_quantity_{0};
    std::list<Order*> orders_;
};