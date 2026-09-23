#include "book.hpp"
#include "limit.hpp"
#include "order.hpp"
#include "pricelevels.hpp"
#include "trade.hpp"

#include <memory>
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

Quantity Book::cancel_order(OrderId id) {
    const auto it = orders_.find(id);

    if (it == orders_.end()) {
        return 0;
    }

    Order& order = *it->second;

    PriceLevels& lvls = order.side() == Side::Buy ? bids_ : asks_;

    Limit* limit = lvls.find(order.price());

    if (limit == nullptr) {
        return 0;
    }

    const Quantity remaining = order.remaining_quantity();

    if (!limit->remove_order(id)) {
        return 0;
    }

    if (limit->empty()) {
        lvls.erase(order.price());
    }

    orders_.erase(it);

    return remaining;
}

std::vector<Trade> Book::execute_limit_order(OrderId id, Side side, Price price, Quantity quantity) {
    if (orders_.contains(id)) {
        throw std::invalid_argument("Order ID already exists\n");
    }

    Order incoming(id, side, price, quantity);

    std::vector<Trade> trades;

    PriceLevels& opposite = side == Side::Buy ? asks_ : bids_;

    while (incoming.remaining_quantity() > 0 && opposite.size()) {
        
        Limit& level = opposite.best();

        const bool crosses = side == Side::Buy 
        ? level.price() <= incoming.price() 
        : level.price () >= incoming.price();

        if (!crosses) break;

        Order& maker = level.front();

        const Quantity quantity_traded = std::min(incoming.remaining_quantity(), maker.remaining_quantity());

        maker.fill(quantity_traded);
        incoming.fill(quantity_traded);

        trades.push_back({maker.id(), incoming.id(), maker.price(), quantity_traded});

        if (maker.is_filled()) {
            const OrderId maker_id = maker.id();
            const Price maker_price = level.price();

            level.remove_front();
            orders_.erase(maker_id);

            if (level.empty()) {
                opposite.erase(maker_price);
            }
        }
    }

    if (incoming.remaining_quantity() > 0) {
        auto resting = std::make_unique<Order>(std::move(incoming));

        Order& reference = *resting;

        PriceLevels& own = side == Side::Buy ? bids_ : asks_;

        Limit& level = own.get_or_create(price);
        orders_.emplace(id, std::move(resting));
        level.add_order(reference);
    }
    return trades;
}