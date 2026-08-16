#include "book.hpp"
#include "order.hpp"
#include "limit.hpp"
#include <cstddef>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <random>


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

Limit* Book::getBuyTree() const {
    return buyTree;
}

Limit* Book::getSellTree() const {
    return sellTree;
}

Limit* Book::getLowestSell() const {
    return lowestSell;
}

Limit* Book::getHighestBuy() const {
    return highestBuy;
}

Limit* Book::getStopBuyTree() const {
    return stopBuyTree;
}

Limit* Book::getStopSellTree() const {
    return stopSellTree;
}

Limit* Book::getHighestStopSell() const {
    return highestStopSell;
}

Limit* Book::getLowestStopBuy() const {
    return lowestStopBuy;
}

// execute order
void Book::marketOrder(int orderId, bool buyOrSell, int shares) {
    executedOrdersCount = 0;
    AVLTreeBalanceCount = 0;

    marketOrderHelper(orderId, buyOrSell, shares);
    executeStopOrders(buyOrSell);
}

// add a new limit order 
void Book::addLimitOrder(int orderId, bool buyOrSell, int shares, int limitPrice) {
    AVLTreeBalanceCount = 0;
    // if order is executed imediately
    shares = limitOrderAsMarketOrder(orderId, buyOrSell, shares, limitPrice);

    if (shares != 0) {
        Order* newOrder = new Order(orderId, buyOrSell, shares, limitPrice);
        orderMap.emplace(orderId, newOrder);
        auto &limitMap = buyOrSell ? limitBuyMap : limitSellMap;

        if (limitMap.find(limitPrice) == limitMap.end()) {
            addLimit(limitPrice, newOrder->getBuyOrSell());
        }

        limitMap.at(limitPrice)->append(newOrder);
    }
    else {
        executeStopOrders(buyOrSell);
    }
}

// delete limit order from book
void Book::cancelLimitOrder(int orderId) {
    executedOrdersCount = 0;
    AVLTreeBalanceCount = 0;
    Order* order = searchOrderMap(orderId);
    // Order* order;
    if (order != nullptr) {
        order->cancel();
        if (order->getParentLimit()->getSize() == 0) {
            deleteLimit(order->getParentLimit());
        }
        deleteFromOrderMap(orderId);
        delete order;
    }
}

void Book::modifyLimitOrder(int orderId, int newShares, int newLimit) {
    executedOrdersCount = 0;
    AVLTreeBalanceCount = 0;
    Order* order = searchOrderMap(orderId);
    if (order != nullptr) {
        order->cancel();
        if (order->getParentLimit()->getSize() == 0) {
            deleteLimit(order->getParentLimit());
        }
        order->modifyOrder(newShares, newLimit);
        auto& limitMap = order->getBuyOrSell() ? limitBuyMap : limitSellMap;

        if (limitMap.find(newLimit) == limitMap.end()) {
            addLimit(newLimit, order->getBuyOrSell());
        }
        limitMap.at(newLimit)->append(order);
    }
}

void Book::addStopOrder(int orderId, bool buyOrSell, int shares, int stopPrice) {
    executedOrdersCount = 0;
    AVLTreeBalanceCount = 0;
    // immediate execution of stop orders
    shares = stopOrderAsMarketOrder(orderId, buyOrSell, shares, stopPrice);

    if (shares != 0) {
        Order* newOrder = new Order(orderId, buyOrSell, shares , 0);
        orderMap.emplace(orderId, newOrder);

        if (stopMap.find(stopPrice) == stopMap.end()) {
            addStop(stopPrice, newOrder->getBuyOrSell());
        }
        stopMap.at(stopPrice)->append(newOrder);
    }
}

// delete order from stopbook
void Book::canceLStopOrder(int orderId) {
    executedOrdersCount = 0;
    AVLTreeBalanceCount = 0;
    Order* order = searchOrderMap(orderId);

    if (order != nullptr) {
        order->cancel();
        if (order->getParentLimit()->getSize() == 0) {
            deleteStopLevel(order->getParentLimit());
        }
        deleteFromOrderMap(orderId);
        delete order;
    }
}

// modify existing stoporder
void Book::modifyStopOrder(int orderId, int newShares, int newStopPrice) {
    executedOrdersCount = 0;
    AVLTreeBalanceCount = 0;
    Order* order = searchOrderMap(orderId);

    if (order != nullptr) {
        order->cancel();
        if (order->getParentLimit()->getSize() == 0) {
            deleteStopLevel(order->getParentLimit());
        }
        order->modifyOrder(newShares, 0);
        if (stopMap.find(newStopPrice) == stopMap.end()) {
            addStop(newStopPrice, order->getBuyOrSell());
        }
        stopMap.at(newStopPrice)->append(order);
    }
}

// add stop limit order
void Book::addStopLimitOrder(int orderId, bool buyOrSell, int shares, int limitPrice, int stopPrice) {
    executedOrdersCount = 0;
    AVLTreeBalanceCount = 0;
    // order executed immediately
    shares = stopLimitOrderAsLimitOrder(orderId, buyOrSell, shares, limitPrice, stopPrice);
    if (shares != 0) {
        Order* newOrder = new Order(orderId, buyOrSell, shares, limitPrice);
        orderMap.emplace(orderId, newOrder);

        if (stopMap.find(stopPrice) == stopMap.end()) {
            addStop(stopPrice, newOrder->getBuyOrSell());
        }

    }
}

void Book::cancelStopLimitOrder(int orderId) {
    executedOrdersCount = 0;
    AVLTreeBalanceCount = 0;
    Order* order = searchOrderMap(orderId);

    if (order != nullptr) {
        order->cancel();

        if (order->getParentLimit()->getSize() == 0) {
            deleteStopLevel(order->getParentLimit());
        }
        deleteFromOrderMap(orderId);
        delete order;
    }
}

//modify existing stoplimitorder
void Book::modifyStopLimitOrder(int orderId, int newShares, int newLimitPrice, int newStopPrice) {
    executedOrdersCount = 0;
    AVLTreeBalanceCount = 0;
    Order* order = searchOrderMap(orderId);

    if (order != nullptr) {
        order->cancel();
        if (order->getParentLimit()->getSize() == 0) {
            deleteStopLevel(order->getParentLimit());
        }
        order->modifyOrder(newShares, newLimitPrice);

        if (stopMap.find(newStopPrice) == stopMap.end()) {
            addStop(newStopPrice, order->getBuyOrSell());
        }
        stopMap.at(newStopPrice)->append(order);
    }
}

//  get tree height of limit
int Book::getLimitHeight(Limit* limit) const {
    if (limit == NULL) {
        return 0;
    }

    int l = getLimitHeight(limit->getLeftChild());
    int r = getLimitHeight(limit->getRightChild());
    return std::max(l, r) + 1;
}

// search map to find order
Order* Book::searchOrderMap(int orderId) const {
    auto it = orderMap.find(orderId);
    if (it != orderMap.end()) {
        return it->second;
    }
    std::cout << "NO order with this number : " << orderId << std::endl;
    return nullptr;
}

// search for limit
Limit* Book::searchLimitMaps(int limitPrice, bool buyOrSell) const {
    auto& limitMap = buyOrSell ? limitBuyMap : limitSellMap;

    auto it = limitMap.find(limitPrice);
    if (it != limitMap.end()) {
        return it->second;
    }
    std::cout << "No " << (buyOrSell ? "buy" : "sell") << " Limit at " << limitPrice << std::endl;
    return nullptr;
}

// search for stop lvl
Limit* Book::searchStopMap(int stopPrice) const {
    auto it = stopMap.find(stopPrice);
    if (it != stopMap.end()) {
        return it->second;
    }
    std::cout << "No stop lvl at " << stopPrice << std::endl;
    return nullptr;
}

Order* Book::getRandomOrder(int key, std::mt19937 gen) const {
    if (!key) {
        if (limitOrders.size() > 10000) {
            std::uniform_int_distribution<> mapDist(0, limitOrders.size() -1 );
            int randIndex = mapDist(gen);

            auto it = limitOrders.begin();
            std::advance(it, randIndex);
            return *it;
        }
        return nullptr;
    }
    else if (key == 1) {
        if (stopOrders.size() > 500) {
            std::uniform_int_distribution<> mapDist(0, stopOrders.size() -1 );
            int randIndex = mapDist(gen);

            auto it = stopOrders.begin();
            std::advance(it, randIndex);
            return *it;
        }
        return nullptr;
    }
    else if (key == 2) {
        if (stopLimitOrders.size() > 500) {
            std::uniform_int_distribution<> mapDist(0, stopLimitOrders.size() - 1);
            int randIndex = mapDist(gen);

            auto it = stopLimitOrders.begin();
            std:std::advance(it, randIndex);
            return *it;
        }
        return nullptr;
    }
    return nullptr;
}

