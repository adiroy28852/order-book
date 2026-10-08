#include "book.hpp"
#include "limit.hpp"
#include "order.hpp"
#include "pricelevels.hpp"
#include "trade.hpp"

#include <algorithm>
#include <limits>
#include <memory>
#include <stdexcept>
#include <unordered_set>
#include <utility>

Order *Book::find_order(OrderId id) noexcept {
  const auto it = orders_.find(id);

  if (it == orders_.end())
    return nullptr;

  return it->second.get();
}

const Order *Book::find_order(OrderId id) const noexcept {
  const auto it = orders_.find(id);

  if (it == orders_.end())
    return nullptr;

  return it->second.get();
}

void Book::assert_invariants() const {
  std::unordered_set<OrderId> indexed_ids;
  indexed_ids.reserve(orders_.size());

  const auto check_side = [this, &indexed_ids](const PriceLevels& side_levels,
                                               Side expected_side) {
    for (const auto& [price, level] : side_levels.levels()) {
      if (level.empty()) {
        throw std::logic_error("Empty price level in book");
      }

      Quantity total = 0;
      for (const Order* order : level.orders()) {
        if (order == nullptr || order->side() != expected_side ||
            order->price() != price || order->is_filled()) {
          throw std::logic_error("Order does not match its price level");
        }

        const auto owner = orders_.find(order->id());
        if (owner == orders_.end() || owner->second.get() != order) {
          throw std::logic_error("Price level contains an unowned order");
        }
        if (!indexed_ids.insert(order->id()).second) {
          throw std::logic_error("Order appears in multiple price levels");
        }

        if (order->remaining_quantity() >
            std::numeric_limits<Quantity>::max() - total) {
          throw std::logic_error("Price-level quantity overflow");
        }
        total += order->remaining_quantity();
      }

      if (total != level.total_quantity()) {
        throw std::logic_error("Price-level aggregate quantity is incorrect");
      }
    }
  };

  check_side(bids_, Side::Buy);
  check_side(asks_, Side::Sell);

  if (indexed_ids.size() != orders_.size()) {
    throw std::logic_error("Order registry and price levels disagree");
  }
  for (const auto& [id, order] : orders_) {
    if (order == nullptr || order->id() != id ||
        indexed_ids.find(id) == indexed_ids.end()) {
      throw std::logic_error("Order registry contains an unindexed order");
    }
  }
}

Order &Book::add_limit_order(OrderId id, Side side, Price price,
                             Quantity quantity) {
  if (orders_.contains(id)) {
    throw std::invalid_argument("Order ID already exists\n");
  }

  auto order = std::make_unique<Order>(id, side, price, quantity);
  Order &reference = *order;
  // first validate get/create limit, create order, then emplace into orders_.
  PriceLevels &lvls = side == Side::Buy ? bids_ : asks_;

  Limit &limit = lvls.get_or_create(price);

  orders_.emplace(id, std::move(order));
  limit.add_order(reference);

  return reference;
}

const PriceLevels &Book::bids() const noexcept { return bids_; }

const PriceLevels &Book::asks() const noexcept { return asks_; }

Quantity Book::cancel_order(OrderId id) {
  const auto it = orders_.find(id);

  if (it == orders_.end()) {
    return 0;
  }

  Order &order = *it->second;

  PriceLevels &lvls = order.side() == Side::Buy ? bids_ : asks_;

  Limit *limit = lvls.find(order.price());

  if (limit == nullptr) {
    throw std::logic_error("Order exists but no price level");
  }

  const Quantity remaining = order.remaining_quantity();

  if (!limit->remove_order(id)) {
    throw std::logic_error("Order exists but not in price level");
  }

  if (limit->empty()) {
    lvls.erase(order.price());
  }

  orders_.erase(it);

  return remaining;
}

std::vector<Trade> Book::execute_limit_order(OrderId id, Side side, Price price,
                                             Quantity quantity) {
  if (orders_.contains(id)) {
    throw std::invalid_argument("Order ID already exists");
  }
  Order incoming(id, side, price, quantity);
  Quantity remaining = incoming.remaining_quantity();
  auto trades = match(id, side, remaining, price);

  if (remaining > 0) {
    const Quantity executed = incoming.remaining_quantity() - remaining;
    if (executed > 0) incoming.fill(executed);
    auto resting = std::make_unique<Order>(std::move(incoming));

    Order &reference = *resting;

    PriceLevels &own = side == Side::Buy ? bids_ : asks_;

    Limit &level = own.get_or_create(price);
    orders_.emplace(id, std::move(resting));
    level.add_order(reference);
  }
  return trades;
}

std::vector<Trade> Book::execute_market_order(OrderId id, Side side,
                                              Quantity quantity) {
  if (id == 0) throw std::invalid_argument("OrderId cannot be 0");
  if (quantity == 0) throw std::invalid_argument("Order quantity must be non-zero");
  if (orders_.contains(id)) throw std::invalid_argument("Order ID already exists");

  Quantity remaining = quantity;
  return match(id, side, remaining, std::nullopt);
}

std::vector<Trade> Book::match(OrderId taker_id, Side side,
                               Quantity& remaining,
                               std::optional<Price> limit_price) {
  std::vector<Trade> trades;
  PriceLevels &opposite = side == Side::Buy ? asks_ : bids_;

  while (remaining > 0 && !opposite.empty()) {
    Limit &level = opposite.best();

    if (limit_price) {
      const bool crosses = side == Side::Buy
                               ? level.price() <= *limit_price
                               : level.price() >= *limit_price;
      if (!crosses) break;
    }

    Order& maker = level.front();
    const OrderId maker_id = maker.id();
    const Price trade_price = maker.price();
    const Quantity quantity_traded = std::min(remaining, maker.remaining_quantity());
    const bool maker_will_be_filled = quantity_traded == maker.remaining_quantity();

    const Quantity executed = level.execute_front(quantity_traded);
    remaining -= executed;
    trades.push_back({maker_id, taker_id, trade_price, executed});

    if (maker_will_be_filled) orders_.erase(maker_id);
    if (level.empty()) opposite.erase(trade_price);
  }
  return trades;
}
