#include "order-book/book.hpp"
#include "order-book/limit.hpp"
#include "order-book/order.hpp"
#include "order-book/pricelevels.hpp"

#include <cassert>
#include <iostream>
#include <stdexcept>

void test_order() {
    Order order(1, Side::Buy, 100, 50);

    assert(order.id() == 1);
    assert(order.side() == Side::Buy);
    assert(order.price() == 100);
    assert(order.original_quantity() == 50);
    assert(order.remaining_quantity() == 50);
    assert(!order.is_filled());

    const Quantity filled = order.fill(20);

    assert(filled == 20);
    assert(order.remaining_quantity() == 30);
    assert(!order.is_filled());

    order.fill(30);

    assert(order.remaining_quantity() == 0);
    assert(order.is_filled());
}

void test_invalid_order() {
    bool threw = false;

    try {
        Order order(0, Side::Buy, 100, 10);
    } catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);

    threw = false;

    try {
        Order order(1, Side::Buy, 0, 10);
    } catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);

    threw = false;

    try {
        Order order(1, Side::Buy, 100, 0);
    } catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);
}

void test_invalid_fill() {
    Order order(1, Side::Buy, 100, 50);

    bool threw = false;

    try {
        order.fill(0);
    } catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);

    threw = false;

    try {
        order.fill(51);
    } catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);
}

void test_limit() {
    Limit limit(100);

    Order order1(1, Side::Buy, 100, 50);
    Order order2(2, Side::Buy, 100, 30);

    limit.add_order(order1);
    limit.add_order(order2);

    assert(limit.price() == 100);
    assert(limit.order_count() == 2);
    assert(limit.total_quantity() == 80);
    assert(!limit.empty());

    assert(limit.front().id() == 1);

    const Quantity executed = limit.execute(60);

    assert(executed == 60);
    assert(order1.is_filled());
    assert(order2.remaining_quantity() == 20);
    assert(limit.order_count() == 1);
    assert(limit.total_quantity() == 20);
    assert(limit.front().id() == 2);
}

void test_limit_cancellation() {
    Limit limit(100);

    Order order1(1, Side::Buy, 100, 50);
    Order order2(2, Side::Buy, 100, 30);
    Order order3(3, Side::Buy, 100, 20);

    limit.add_order(order1);
    limit.add_order(order2);
    limit.add_order(order3);

    assert(limit.total_quantity() == 100);
    assert(limit.order_count() == 3);

    assert(limit.remove_order(2));

    assert(limit.total_quantity() == 70);
    assert(limit.order_count() == 2);
    assert(limit.front().id() == 1);

    assert(!limit.remove_order(999));

    assert(limit.remove_order(1));

    assert(limit.total_quantity() == 20);
    assert(limit.order_count() == 1);
    assert(limit.front().id() == 3);
}

void test_price_levels() {
    PriceLevels bids(PriceOrder::Descending);

    bids.get_or_create(100);
    bids.get_or_create(105);
    bids.get_or_create(95);

    assert(bids.size() == 3);
    assert(bids.best().price() == 105);

    assert(bids.find(100) != nullptr);
    assert(bids.find(999) == nullptr);

    bids.erase(105);

    assert(bids.size() == 2);
    assert(bids.best().price() == 100);

    PriceLevels asks(PriceOrder::Ascending);

    asks.get_or_create(100);
    asks.get_or_create(105);
    asks.get_or_create(95);

    assert(asks.size() == 3);
    assert(asks.best().price() == 95);
}

void test_book_insertion() {
    Book book;

    auto& order1 = book.add_limit_order(1, Side::Buy, 100, 50);
    auto& order2 = book.add_limit_order(2, Side::Buy, 100, 30);
    auto& order3 = book.add_limit_order(3, Side::Buy, 105, 20);

    assert(order1.id() == 1);
    assert(order2.id() == 2);
    assert(order3.id() == 3);

    assert(book.find_order(1) != nullptr);
    assert(book.find_order(2) != nullptr);
    assert(book.find_order(3) != nullptr);
    assert(book.find_order(999) == nullptr);

    assert(book.bids().size() == 2);
    assert(book.bids().best().price() == 105);
    assert(book.bids().best().total_quantity() == 20);

    assert(book.bids().find(100) != nullptr);
    assert(book.bids().find(100)->order_count() == 2);
    assert(book.bids().find(100)->total_quantity() == 80);
    assert(book.bids().find(100)->front().id() == 1);

    auto& ask1 = book.add_limit_order(4, Side::Sell, 110, 40);
    auto& ask2 = book.add_limit_order(5, Side::Sell, 105, 20);
    auto& ask3 = book.add_limit_order(6, Side::Sell, 110, 10);

    assert(ask1.id() == 4);
    assert(ask2.id() == 5);
    assert(ask3.id() == 6);

    assert(book.asks().size() == 2);
    assert(book.asks().best().price() == 105);
    assert(book.asks().best().total_quantity() == 20);

    assert(book.asks().find(110) != nullptr);
    assert(book.asks().find(110)->order_count() == 2);
    assert(book.asks().find(110)->total_quantity() == 50);
}

void test_duplicate_order_id() {
    Book book;

    book.add_limit_order(1, Side::Buy, 100, 50);

    bool threw = false;

    try {
        book.add_limit_order(1, Side::Buy, 200, 10);
    } catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);

    assert(book.find_order(1) != nullptr);
    assert(book.find_order(1)->price() == 100);
    assert(book.find_order(1)->remaining_quantity() == 50);
}

void test_order_address_stability() {
    Book book;

    auto& first = book.add_limit_order(1, Side::Buy, 100, 50);
    Order* first_address = &first;

    for (OrderId id = 1000; id < 10000; ++id) {
        book.add_limit_order(id, Side::Buy, 200, 1);
    }

    assert(book.find_order(1) == first_address);
    assert(book.find_order(1)->id() == 1);

    assert(book.bids().find(100) != nullptr);
    assert(book.bids().find(100)->front().id() == 1);
}

void test_cancel_order() {
    Book book;

    book.add_limit_order(1, Side::Buy, 100, 50);
    book.add_limit_order(2, Side::Buy, 100, 30);
    book.add_limit_order(3, Side::Buy, 105, 20);

    const Quantity cancelled = book.cancel_order(2);

    assert(cancelled == 30);
    assert(book.find_order(2) == nullptr);

    assert(book.bids().find(100) != nullptr);
    assert(book.bids().find(100)->order_count() == 1);
    assert(book.bids().find(100)->total_quantity() == 50);
    assert(book.bids().find(100)->front().id() == 1);
}

void test_cancel_last_order_at_price() {
    Book book;

    book.add_limit_order(1, Side::Buy, 100, 50);
    book.add_limit_order(2, Side::Buy, 105, 20);

    assert(book.bids().find(100) != nullptr);

    const Quantity cancelled = book.cancel_order(1);

    assert(cancelled == 50);
    assert(book.find_order(1) == nullptr);
    assert(book.bids().find(100) == nullptr);

    assert(book.bids().size() == 1);
    assert(book.bids().best().price() == 105);
}

void test_cancel_nonexistent_order() {
    Book book;

    book.add_limit_order(1, Side::Buy, 100, 50);

    assert(book.cancel_order(999) == 0);

    assert(book.find_order(1) != nullptr);
    assert(book.bids().size() == 1);
}

void test_cancel_partially_filled_order() {
    Book book;
    book.add_limit_order(1, Side::Sell, 100, 100);
    const auto trades = book.execute_limit_order(2, Side::Buy, 100, 40);
    assert(trades.size() == 1);
    assert(book.cancel_order(1) == 60);
    assert(book.find_order(1) == nullptr);
    assert(book.asks().empty());
    book.assert_invariants();
}

void test_cancel_middle_preserves_fifo() {
    Book book;
    book.add_limit_order(1, Side::Sell, 100, 10);
    book.add_limit_order(2, Side::Sell, 100, 10);
    book.add_limit_order(3, Side::Sell, 100, 10);
    assert(book.cancel_order(2) == 10);

    const auto trades = book.execute_limit_order(4, Side::Buy, 100, 15);
    assert(trades.size() == 2);
    assert(trades[0].maker_order_id == 1 && trades[0].quantity == 10);
    assert(trades[1].maker_order_id == 3 && trades[1].quantity == 5);
    assert(book.find_order(3)->remaining_quantity() == 5);
    book.assert_invariants();
}

void test_match_after_partial_fill() {
    Book book;
    book.add_limit_order(1, Side::Sell, 100, 100);
    assert(book.execute_limit_order(2, Side::Buy, 100, 40).size() == 1);
    assert(book.execute_market_order(3, Side::Buy, 30).size() == 1);
    assert(book.find_order(1)->remaining_quantity() == 30);
    book.assert_invariants();
}

void test_book_rejects_invalid_inputs() {
    Book book;
    const auto must_throw = [](auto&& operation) {
        bool threw = false;
        try { operation(); } catch (const std::invalid_argument&) { threw = true; }
        assert(threw);
    };
    must_throw([&] { book.add_limit_order(0, Side::Buy, 100, 1); });
    must_throw([&] { book.add_limit_order(1, Side::Buy, 0, 1); });
    must_throw([&] { book.add_limit_order(1, Side::Buy, 100, 0); });
    book.add_limit_order(1, Side::Buy, 100, 1);
    must_throw([&] { book.execute_limit_order(1, Side::Buy, 100, 1); });
    must_throw([&] { book.execute_market_order(1, Side::Buy, 1); });
    must_throw([&] { book.execute_market_order(2, Side::Buy, 0); });
    book.assert_invariants();
}

void test_book_invariants_across_operations() {
    Book book;
    book.assert_invariants();
    book.add_limit_order(1, Side::Sell, 100, 25);
    book.assert_invariants();
    book.add_limit_order(2, Side::Sell, 105, 25);
    book.assert_invariants();
    book.execute_limit_order(3, Side::Buy, 105, 30);
    book.assert_invariants();
    book.cancel_order(2);
    book.assert_invariants();
    book.execute_market_order(4, Side::Sell, 5);
    book.assert_invariants();
}

void test_limit_match_exact_price() {
    Book book;

    book.add_limit_order(1, Side::Sell, 100, 40);

    const auto trades =
        book.execute_limit_order(2, Side::Buy, 100, 40);

    assert(trades.size() == 1);

    assert(trades[0].maker_order_id == 1);
    assert(trades[0].taker_order_id == 2);
    assert(trades[0].price == 100);
    assert(trades[0].quantity == 40);

    assert(book.find_order(1) == nullptr);
    assert(book.find_order(2) == nullptr);

    assert(book.asks().empty());
}

void test_limit_order_does_not_cross() {
    Book book;

    book.add_limit_order(1, Side::Sell, 105, 50);

    const auto trades =
        book.execute_limit_order(2, Side::Buy, 100, 20);

    assert(trades.empty());

    assert(book.find_order(1) != nullptr);
    assert(book.find_order(2) != nullptr);

    assert(book.find_order(1)->remaining_quantity() == 50);
    assert(book.find_order(2)->remaining_quantity() == 20);

    assert(book.bids().best().price() == 100);
    assert(book.asks().best().price() == 105);
}

void test_partial_maker_fill() {
    Book book;

    book.add_limit_order(1, Side::Sell, 100, 100);

    const auto trades =
        book.execute_limit_order(2, Side::Buy, 100, 30);

    assert(trades.size() == 1);

    assert(trades[0].maker_order_id == 1);
    assert(trades[0].taker_order_id == 2);
    assert(trades[0].price == 100);
    assert(trades[0].quantity == 30);

    assert(book.find_order(1) != nullptr);
    assert(book.find_order(1)->remaining_quantity() == 70);

    assert(book.find_order(2) == nullptr);

    assert(book.asks().find(100) != nullptr);
    assert(book.asks().find(100)->total_quantity() == 70);
}

void test_partial_taker_fill() {
    Book book;

    book.add_limit_order(1, Side::Sell, 100, 40);
    book.add_limit_order(2, Side::Sell, 105, 30);

    const auto trades =
        book.execute_limit_order(3, Side::Buy, 105, 50);

    assert(trades.size() == 2);

    assert(trades[0].maker_order_id == 1);
    assert(trades[0].price == 100);
    assert(trades[0].quantity == 40);

    assert(trades[1].maker_order_id == 2);
    assert(trades[1].price == 105);
    assert(trades[1].quantity == 10);

    assert(book.find_order(1) == nullptr);

    assert(book.find_order(2) != nullptr);
    assert(book.find_order(2)->remaining_quantity() == 20);

    assert(book.find_order(3) == nullptr);

    assert(book.asks().find(100) == nullptr);
    assert(book.asks().find(105) != nullptr);
    assert(book.asks().find(105)->total_quantity() == 20);
}

void test_fifo_matching() {
    Book book;

    book.add_limit_order(1, Side::Sell, 100, 40);
    book.add_limit_order(2, Side::Sell, 100, 30);
    book.add_limit_order(3, Side::Sell, 100, 20);

    const auto trades =
        book.execute_limit_order(4, Side::Buy, 100, 50);

    assert(trades.size() == 2);

    assert(trades[0].maker_order_id == 1);
    assert(trades[0].quantity == 40);

    assert(trades[1].maker_order_id == 2);
    assert(trades[1].quantity == 10);

    assert(book.find_order(1) == nullptr);

    assert(book.find_order(2) != nullptr);
    assert(book.find_order(2)->remaining_quantity() == 20);

    assert(book.find_order(3) != nullptr);
    assert(book.find_order(3)->remaining_quantity() == 20);

    assert(book.asks().best().front().id() == 2);
}

void test_multi_level_matching() {
    Book book;

    book.add_limit_order(1, Side::Sell, 100, 40);
    book.add_limit_order(2, Side::Sell, 105, 30);
    book.add_limit_order(3, Side::Sell, 110, 20);

    const auto trades =
        book.execute_limit_order(4, Side::Buy, 107, 60);

    assert(trades.size() == 2);

    assert(trades[0].maker_order_id == 1);
    assert(trades[0].price == 100);
    assert(trades[0].quantity == 40);

    assert(trades[1].maker_order_id == 2);
    assert(trades[1].price == 105);
    assert(trades[1].quantity == 20);

    assert(book.find_order(1) == nullptr);

    assert(book.find_order(2) != nullptr);
    assert(book.find_order(2)->remaining_quantity() == 10);

    assert(book.find_order(3) != nullptr);
    assert(book.find_order(3)->remaining_quantity() == 20);

    assert(book.asks().find(100) == nullptr);
    assert(book.asks().find(105) != nullptr);
    assert(book.asks().find(110) != nullptr);

    assert(book.asks().find(105)->total_quantity() == 10);
    assert(book.asks().find(110)->total_quantity() == 20);
}

void test_buy_matching() {
    Book book;

    book.add_limit_order(1, Side::Sell, 100, 20);

    const auto trades =
        book.execute_limit_order(2, Side::Buy, 105, 20);

    assert(trades.size() == 1);
    assert(trades[0].maker_order_id == 1);
    assert(trades[0].taker_order_id == 2);
    assert(trades[0].price == 100);
    assert(trades[0].quantity == 20);

    assert(book.asks().empty());
    assert(book.find_order(1) == nullptr);
    assert(book.find_order(2) == nullptr);
}

void test_sell_matching() {
    Book book;

    book.add_limit_order(1, Side::Buy, 100, 20);

    const auto trades =
        book.execute_limit_order(2, Side::Sell, 95, 20);

    assert(trades.size() == 1);
    assert(trades[0].maker_order_id == 1);
    assert(trades[0].taker_order_id == 2);
    assert(trades[0].price == 100);
    assert(trades[0].quantity == 20);

    assert(book.bids().empty());
    assert(book.find_order(1) == nullptr);
    assert(book.find_order(2) == nullptr);
}

void test_sell_does_not_cross() {
    Book book;

    book.add_limit_order(1, Side::Buy, 100, 20);

    const auto trades =
        book.execute_limit_order(2, Side::Sell, 105, 10);

    assert(trades.empty());

    assert(book.find_order(1) != nullptr);
    assert(book.find_order(2) != nullptr);

    assert(book.find_order(1)->remaining_quantity() == 20);
    assert(book.find_order(2)->remaining_quantity() == 10);

    assert(book.bids().best().price() == 100);
    assert(book.asks().best().price() == 105);
}

void test_remainder_becomes_resting_order() {
    Book book;

    book.add_limit_order(1, Side::Sell, 100, 30);

    const auto trades =
        book.execute_limit_order(2, Side::Buy, 105, 50);

    assert(trades.size() == 1);

    assert(trades[0].quantity == 30);

    assert(book.find_order(1) == nullptr);

    assert(book.find_order(2) != nullptr);
    assert(book.find_order(2)->remaining_quantity() == 20);
    assert(book.find_order(2)->side() == Side::Buy);
    assert(book.find_order(2)->price() == 105);

    assert(book.bids().best().price() == 105);
    assert(book.bids().best().total_quantity() == 20);
}

void test_empty_price_level_removal_after_match() {
    Book book;

    book.add_limit_order(1, Side::Sell, 100, 10);
    book.add_limit_order(2, Side::Sell, 105, 10);

    assert(book.asks().size() == 2);

    const auto trades =
        book.execute_limit_order(3, Side::Buy, 100, 10);

    assert(trades.size() == 1);

    assert(book.asks().find(100) == nullptr);
    assert(book.asks().find(105) != nullptr);
    assert(book.asks().size() == 1);
}

void test_market_buy() {
    Book book;

    book.add_limit_order(1, Side::Sell, 100, 20);
    book.add_limit_order(2, Side::Sell, 105, 30);

    const auto trades =
        book.execute_market_order(3, Side::Buy, 40);

    assert(trades.size() == 2);

    assert(trades[0].maker_order_id == 1);
    assert(trades[0].taker_order_id == 3);
    assert(trades[0].price == 100);
    assert(trades[0].quantity == 20);

    assert(trades[1].maker_order_id == 2);
    assert(trades[1].taker_order_id == 3);
    assert(trades[1].price == 105);
    assert(trades[1].quantity == 20);

    assert(book.find_order(1) == nullptr);

    assert(book.find_order(2) != nullptr);
    assert(book.find_order(2)->remaining_quantity() == 10);

    assert(book.asks().find(100) == nullptr);
    assert(book.asks().find(105) != nullptr);
    assert(book.asks().find(105)->total_quantity() == 10);

    assert(book.find_order(3) == nullptr);
}

void test_market_sell() {
    Book book;

    book.add_limit_order(1, Side::Buy, 105, 30);
    book.add_limit_order(2, Side::Buy, 100, 20);

    const auto trades =
        book.execute_market_order(3, Side::Sell, 40);

    assert(trades.size() == 2);

    assert(trades[0].maker_order_id == 1);
    assert(trades[0].price == 105);
    assert(trades[0].quantity == 30);

    assert(trades[1].maker_order_id == 2);
    assert(trades[1].price == 100);
    assert(trades[1].quantity == 10);

    assert(book.find_order(1) == nullptr);

    assert(book.find_order(2) != nullptr);
    assert(book.find_order(2)->remaining_quantity() == 10);

    assert(book.bids().find(105) == nullptr);
    assert(book.bids().find(100) != nullptr);
    assert(book.bids().find(100)->total_quantity() == 10);
}

void test_market_order_exhausts_liquidity() {
    Book book;

    book.add_limit_order(1, Side::Sell, 100, 20);

    const auto trades =
        book.execute_market_order(2, Side::Buy, 50);

    assert(trades.size() == 1);

    assert(trades[0].maker_order_id == 1);
    assert(trades[0].taker_order_id == 2);
    assert(trades[0].price == 100);
    assert(trades[0].quantity == 20);

    assert(book.asks().empty());
    assert(book.find_order(1) == nullptr);
    assert(book.find_order(2) == nullptr);
}

void test_market_order_empty_book() {
    Book book;

    const auto trades =
        book.execute_market_order(1, Side::Buy, 50);

    assert(trades.empty());
    assert(book.bids().empty());
    assert(book.asks().empty());
    assert(book.find_order(1) == nullptr);
}

int main() {
    test_order();
    test_invalid_order();
    test_invalid_fill();

    test_limit();
    test_limit_cancellation();

    test_price_levels();

    test_book_insertion();
    test_duplicate_order_id();
    test_order_address_stability();

    test_cancel_order();
    test_cancel_last_order_at_price();
    test_cancel_nonexistent_order();
    test_cancel_partially_filled_order();
    test_cancel_middle_preserves_fifo();
    test_match_after_partial_fill();
    test_book_rejects_invalid_inputs();
    test_book_invariants_across_operations();

    test_limit_match_exact_price();
    test_limit_order_does_not_cross();
    test_partial_maker_fill();
    test_partial_taker_fill();
    test_fifo_matching();
    test_multi_level_matching();
    test_buy_matching();
    test_sell_matching();
    test_sell_does_not_cross();
    test_remainder_becomes_resting_order();
    test_empty_price_level_removal_after_match();
    
    test_market_buy();
    test_market_sell();
    test_market_order_exhausts_liquidity();
    test_market_order_empty_book();

    std::cout << "All tests passed\n";
}
