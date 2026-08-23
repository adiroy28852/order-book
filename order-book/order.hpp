#pragma once
#include <cstdint>

enum class Side : std::uint8_t {
    Buy,
    Sell
};

using OrderId = std::uint64_t;
using Price = std::int64_t;
using Quantity = std::uint64_t;

class Order {
public:
    Order(OrderId id, Side side, Price price, Quantity quantity);

    Order(const Order&) = default;
    Order(Order&&) noexcept = default;

    Order& operator=(const Order&) = default;
    Order& operator=(Order&&) noexcept = default;

    ~Order() = default;

    [[nodiscard]] OrderId id() const noexcept;
    [[nodiscard]] Side side() const noexcept;
    [[nodiscard]] Price price() const noexcept;
    [[nodiscard]] Quantity original_quantity() const noexcept;
    [[nodiscard]] Quantity remaining_quantity() const noexcept;
    
    [[nodiscard]] bool is_filled() const noexcept;

    Quantity fill(Quantity quantity) noexcept;

private:
    OrderId id_;
    Side side_;
    Price price_;
    Quantity original_quantity_;
    Quantity remaining_quantity_;
};