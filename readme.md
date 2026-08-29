# Order Book

A limit order book implemented from scratch in modern C++20.

The project is focused on understanding the data structures, ownership model, matching logic, and performance characteristics behind an electronic order book rather than copying an existing implementation.

## Current Architecture

```text
                    Book
                     │
          ┌──────────┴──────────┐
          │                     │
        bids                   asks
          │                     │
    PriceLevels            PriceLevels
          │                     │
        Limit                 Limit
          │                     │
       Order*                Order*
          │                     │
          └──────────┬──────────┘
                     │
                   Order
```

`Book` owns the actual `Order` objects.

`Limit` does not own orders. It maintains references to orders belonging to `Book`.

This separates:

* Order lifetime
* Price-level organization
* Side-specific price ordering
* Book-level operations

## Data Structures

### Order

Represents an individual order.

An order contains:

* Order ID
* Side
* Price
* Original quantity
* Remaining quantity
* Order state

Orders are owned by `Book` through:

```cpp
std::unordered_map<OrderId, std::unique_ptr<Order>>
```

This provides stable `Order` object addresses even when the unordered map rehashes.

### Limit

Represents all orders at one price.

```text
Limit
├── price
├── total quantity
└── FIFO queue
    ├── Order*
    ├── Order*
    └── Order*
```

Orders at the same price follow price-time priority.

### PriceLevels

Provides the price-level abstraction.

The current implementation uses:

```cpp
std::map<Price, Limit>
```

The ordering is configurable:

```cpp
PriceLevels bids{PriceOrder::Descending};
PriceLevels asks{PriceOrder::Ascending};
```

Therefore:

```text
Bids → highest price is best
Asks → lowest price is best
```

The underlying container is intentionally abstracted behind `PriceLevels` so the implementation can later be replaced with a custom tree.

### Book

`Book` owns:

```text
orders_
bids_
asks_
```

Its responsibility is to coordinate the order book rather than directly manage the internals of individual price levels.

## Ownership Model

The critical ownership relationship is:

```text
Book
 │
 │ owns
 ▼
unique_ptr<Order>
 │
 │ referenced by
 ▼
Limit
 │
 │ stored inside
 ▼
PriceLevels
```

An `Order*` stored by a `Limit` is non-owning.

This prevents the price-level structure from becoming responsible for order lifetime.

## Current Functionality

Currently implemented:

* `Order`
* Order ownership through `std::unique_ptr`
* `Limit`
* FIFO ordering within a price level
* Price-level aggregation
* Bid/ask price ordering
* `PriceLevels` abstraction
* Adding limit orders to `Book`
* Order lookup by ID
* Const-correct access to bid/ask levels
* Basic tests for order insertion and price-level organization

## Example

Adding orders:

```cpp
Book book;

book.add_limit_order(1, Side::Buy, 100, 50);
book.add_limit_order(2, Side::Buy, 100, 30);
book.add_limit_order(3, Side::Buy, 105, 20);

book.add_limit_order(4, Side::Sell, 110, 40);
book.add_limit_order(5, Side::Sell, 105, 20);
```

Produces:

```text
ASK
105 ── 20
110 ── 40

BID
105 ── 20
100 ── 80
```

At price `100`, orders `1` and `2` remain FIFO ordered.

## Matching Engine

Matching is the next major stage.

For an incoming buy:

```text
best ask.price <= incoming buy.price
```

For an incoming sell:

```text
best bid.price >= incoming sell.price
```

Matching will support:

* Full fills
* Partial fills
* Multiple price levels
* Removal of empty price levels
* Remaining quantity becoming a resting order

Example:

```text
ASK
100 × 40
105 × 30

Incoming:
BUY 105 × 50
```

Result:

```text
ASK
105 × 20
```

The `100 × 40` level is completely consumed.

## Planned Architecture

```text
                    Book
                     │
          ┌──────────┴──────────┐
          │                     │
        bids                   asks
          │                     │
    PriceLevels            PriceLevels
          │                     │
     Price Tree              Price Tree
          │                     │
        Limit                 Limit
          │                     │
       Order*                Order*
          │                     │
          └──────────┬──────────┘
                     │
                   Order
```

The long-term intention is to make the price-level storage replaceable.

For example:

```text
PriceLevels
    │
    ├── std::map implementation
    │
    ├── AVL implementation
    │
    └── other ordered-tree implementation
```

The book should depend on the abstraction rather than the specific tree.

## Roadmap

### Phase 1 — Core Structures

* [x] Order
* [x] Limit
* [x] PriceLevels
* [x] Book ownership
* [x] Bid/ask organization
* [x] Basic insertion
* [x] Order lookup

### Phase 2 — Matching (WIP)

* [ ] Match incoming buy orders
* [ ] Match incoming sell orders
* [ ] Full fills
* [ ] Partial fills
* [ ] Multi-level matching
* [ ] Empty-level removal
* [ ] Resting order remainder
* [ ] Matching tests


Benchmark:

* Insert
* Lookup
* Best-price access
* Price-level deletion
* Matching
* Cancellation

The goal is to measure the actual workload rather than assuming a particular tree is faster.

## Design Principles

The project prioritizes:

1. **Correctness before optimization**
2. **Clear ownership**
3. **Explicit data structures**
4. **Modern C++20**
5. **Replaceable components**
6. **Measured performance rather than assumed performance**
7. **Understanding the implementation rather than copying an existing order book**

The current implementation intentionally starts with standard-library structures. Performance-critical replacements should be justified through benchmarks rather than introduced prematurely.

## Build

Compile with C++20:

```bash
clang++ -std=c++20 -Wall -Wextra -Wpedantic \
    -I. \
    main.cpp \
    order-book/order.cpp \
    order-book/limit.cpp \
    order-book/pricelevels.cpp \
    order-book/book.cpp \
    -o order_book
```

Run:

```bash
./order_book
```

Tests:

```bash
clang++ -std=c++20 -Wall -Wextra -Wpedantic \
    -I. \
    tests.cpp \
    order-book/order.cpp \
    order-book/limit.cpp \
    order-book/pricelevels.cpp \
    order-book/book.cpp \
    -o tests
```

```bash
./tests
```

## Project Status

**Current milestone: Core book construction complete.**

The next milestone is the **matching engine**, followed by cancellation and modification. Only after those operations are correct should the underlying price-level data structure be optimized or replaced.
