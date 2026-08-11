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
    for (auto &[id, order] : orderMap) {
        delete order;
    }
    orderMap.clear();

    for (auto &[limitPrice, limit] : limitBuyMap) {
        delete limit;
    }
    limitBuyMap.clear();

    for (auto &[limitPrice, limit] : limitSellMap) {
        delete limit;
    }
    limitSellMap.clear();

    for(auto &[stopPrice, stopLevel] : stopMap) {
        delete stopLevel;
    }
    stopMap.clear();
}