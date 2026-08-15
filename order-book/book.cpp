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

