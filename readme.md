# Order Book

A small in-memory limit order book written from scratch in modern C++20. The v1 goal is a clear, deterministic matching library that demonstrates price-time priority, order ownership, and lifecycle correctness. It is an educational project, not production exchange software.

## Features

- Buy and sell limit orders
- Price-time priority: best price first, FIFO within a price
- Partial and full fills across multiple levels
- Market orders that consume available opposite-side liquidity
- Cancellation by order ID, including partially filled orders
- Trade records for each execution
- Duplicate ID and invalid input rejection
- Empty level cleanup
- Deterministic tests and a book invariant checker

## Design

`Book` owns each live `Order` through `std::unique_ptr` in an ID index. `Limit` stores non-owning pointers in FIFO order and tracks aggregate remaining quantity. `PriceLevels` orders `Limit`s by price using `std::map` (ascending for asks, descending for bids). Matching and order lifetime remain coordinated by `Book`.

A fully filled or cancelled order is removed from both the price level and the ID index. A limit taker with unfilled quantity rests at its limit price. Market taker remainder is discarded when the opposite book has no more liquidity. Trades execute at the maker's resting price.

Cancellation currently searches the FIFO list at its price, so its cost grows with the number of orders at that level. This is accepted for v1; measure before optimizing.

## Example

```cpp
Book book;
book.add_limit_order(1, Side::Sell, 100, 50);
book.add_limit_order(2, Side::Sell, 105, 30);

auto trades = book.execute_limit_order(3, Side::Buy, 100, 20);
// One trade: 20 @ 100, maker order 1, taker order 3.
```

See `main.cpp` for a runnable example. `Book::assert_invariants()` checks that every live order appears exactly once in the correct side/price level and that each level's aggregate quantity is accurate. It throws `std::logic_error` if it finds a mismatch; it is intended as a correctness aid in tests and debug workflows.

## Build and run

From the repository root, with a C++20 compiler:

```sh
c++ -std=c++20 -Wall -Wextra -Wpedantic main.cpp order-book/order.cpp order-book/limit.cpp order-book/pricelevels.cpp order-book/book.cpp -o order_book
./order_book
```

Build and run the deterministic test suite:

```sh
c++ -std=c++20 -Wall -Wextra -Wpedantic tests.cpp order-book/order.cpp order-book/limit.cpp order-book/pricelevels.cpp order-book/book.cpp -o order_book_tests
./order_book_tests
```

## Scope

The v1 engine is single-threaded, in-memory, and uses `std::map` for price levels. It does not provide persistence, exchange connectivity, concurrency, or production execution guarantees. Alternative trees, custom allocators, networking, and kernel-bypass work remain later research directions after correctness and baseline performance are established.
