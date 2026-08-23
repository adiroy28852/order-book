#pragma once

#include "book.hpp"
#include "limit.hpp"
#include "order.hpp"

#include <cstddef>
#include <map>

enum class PriceOrder {
    Ascending, 
    Descending
};

class PriceLevels {
private:
    PriceOrder order_;
    std::map<Price, Limit> levels_;
public:
    explicit PriceLevels(PriceOrder order);

    PriceLevels() = default;
    ~PriceLevels() = default;

    PriceLevels(const PriceLevels&) = delete;
    PriceLevels& operator=(const PriceLevels&) noexcept = default;

    PriceLevels(PriceLevels&&) noexcept = default;
    PriceLevels& operator=(PriceLevels&&) = default;

    Limit& get_or_create(Price price);
    Limit* find(Price price) noexcept;

    void erase(Price price) noexcept;

    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;

    Limit& best();
    const Limit& best() const;
};