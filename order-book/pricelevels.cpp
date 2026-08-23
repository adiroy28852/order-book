#include "pricelevels.hpp"
#include "limit.hpp"
#include "order.hpp"

#include <cstddef>
#include <stdexcept>
#include <tuple>
#include <utility>

PriceLevels::PriceLevels(PriceOrder order) : order_(order) {}

Limit& PriceLevels::get_or_create(Price price) {
    auto it = levels_.find(price);

    if (it != levels_.end()) {
        return it->second;
    }
    auto [inserted_it, inserted] = levels_.emplace(std::piecewise_construct, std::forward_as_tuple(price), std::forward_as_tuple(price));

    return inserted_it->second;
}

Limit* PriceLevels::find(Price price) noexcept {
    const auto it = levels_.find(price);

    if (it == levels_.end()) {
        return NULL;
    }
    return &it->second;
}

void PriceLevels::erase(Price price) noexcept {
    levels_.erase(price);
}

bool PriceLevels::empty() const noexcept {
    return levels_.empty();
}

std::size_t PriceLevels::size() const noexcept {
    return levels_.size();
}

Limit& PriceLevels::best() {
    if (levels_.empty()) {
        throw std::out_of_range("No price levels\n");
    }

    if (order_ == PriceOrder::Ascending) {
        return levels_.begin()->second;
    }
    
    return levels_.rbegin()->second;
}

const Limit& PriceLevels::best() const {
    if (levels_.empty()) {
        throw  std::out_of_range("No price levels \n");
    }

    if (order_ == PriceOrder::Ascending) {
        return levels_.begin()->second;
    }
    
    return levels_.rbegin()->second;
}