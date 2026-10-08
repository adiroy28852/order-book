#pragma once

#include "limit.hpp"

#include <cstddef>
#include <map>

enum class PriceOrder {
    Ascending, 
    Descending
};

class PriceLevels {
    friend class Book;
private:
    PriceOrder order_;
    std::map<Price, Limit> levels_;

    [[nodiscard]] const std::map<Price, Limit>& levels() const noexcept;
public:
    explicit PriceLevels(PriceOrder order);

    // PriceLevels() = default;
    ~PriceLevels() = default;

    PriceLevels(const PriceLevels&) = delete;
    PriceLevels& operator=(const PriceLevels&) = delete;

    PriceLevels(PriceLevels&&) noexcept = default;
    PriceLevels& operator=(PriceLevels&&) noexcept = default;

    Limit& get_or_create(Price price);
    Limit* find(Price price) noexcept;
    const Limit* find(Price price) const noexcept;

    void erase(Price price) noexcept;

    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;

    Limit& best();
    const Limit& best() const;

};
