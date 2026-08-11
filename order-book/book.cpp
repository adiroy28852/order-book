#include "book.hpp"
#include "order.hpp"
#include "limit.hpp"
#include <cstddef>
#include <iostream>
#include <algorithm>


Book::Book() : buyTree(nullptr), sellTree(nullptr), lowestSell(nullptr), 
    highestBuy(nullptr), stopBuyTree(nullptr), stopSellTree(nullptr), 
    highestStopSell(nullptr), lowestStopBuy(nullptr) {}

// destructor ensures garbage collection and freeing of memory
Book::~Book() {
    
}