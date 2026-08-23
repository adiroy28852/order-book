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

// add new order
void Book::addLimit(int limitPrice, bool buyOrSell) {
    auto& limitMap = buyOrSell ? limitBuyMap : limitSellMap;
    auto& tree = buyOrSell ? buyTree : sellTree;
    auto& bookEdge = buyOrSell ? highestBuy : lowestSell;

    Limit* newLimit = new Limit(limitPrice, buyOrSell);
    limitMap.emplace(limitPrice, newLimit);

    if (tree == NULL) {
        tree = newLimit;
        bookEdge = newLimit;
    }
    else {
        Limit* root = insert(tree, newLimit);
        updateBookEdgeInsert(newLimit);
    }
}

// add new stop to book
void Book::addStop(int stopPrice, bool buyOrSell) {
    auto& tree = buyOrSell ? stopBuyTree : stopSellTree;
    auto& bookEdge = buyOrSell ? lowestStopBuy : highestStopSell;

    Limit* newStop = new Limit(stopPrice, buyOrSell);
    stopMap.emplace(stopPrice, newStop);
    if (tree == NULL) {
        tree = newStop;
        bookEdge = newStop;
    }
    else {
        Limit* root = insertStop(tree, newStop);
        updateStopBookEdgeInsert(newStop);
    }
}

// insert limit in the bst
Limit* Book::insert(Limit* root, Limit* limit, Limit* parent) {
    if (root == nullptr) {
        limit->setParent(parent);
        return limit;
    }
    if (limit->getLimitPrice() < root->getLimitPrice()) {
        root->setLeftChild(insert(root->getLeftChild(), limit, root));
        root = balance(root);
    }
    else if (limit->getLimitPrice() > root->getLimitPrice()) {
        root->setRightChild(insert(root->getRightChild(), limit, root));
        root = balance(root);
    }
    return root;
}

// insert limit inthe stop bst
Limit* Book::insertStop(Limit* root, Limit* limit, Limit* parent) {
    if (root == nullptr) {
        limit->setParent(parent);
        return limit;
    }
    if (limit->getLimitPrice() < root->getLimitPrice()) {
        root->setLeftChild(insertStop(root->getLeftChild(), limit, root));
        root = balanceStop(root);
    }
    else if (limit->getLimitPrice() > root->getLimitPrice()) {
        root->setRightChild(insertStop(root->getRightChild(), limit, root));
        root = balanceStop(root);
    }
    return root;
}

//update edge if new limit is on edge of book
void Book::updateBookEdgeInsert(Limit* newLimit) {
    if (newLimit->getBuyOrSell()) {
        if (newLimit->getLimitPrice() > highestBuy->getLimitPrice()) {
            highestBuy = newLimit;
        }
    }
    else {
        if (newLimit->getLimitPrice() < lowestSell->getLimitPrice()) {
            lowestSell = newLimit;
        }
    }
}

//update ege of stopbook if new stop is on edge of book
void Book::updateStopBookEdgeInsert(Limit* newStop) {
    if (newStop->getBuyOrSell()) {
        if (newStop->getLimitPrice() < lowestStopBuy->getLimitPrice()) {
            lowestStopBuy = newStop;
        }
    }
    else {
        if (newStop->getLimitPrice() > highestStopSell->getLimitPrice()) {
            highestStopSell = newStop;
        }
    }
}

// update edge of book if current edge is empty/ied
void Book::updateBookEdgeRemove(Limit* limit) {
    auto& bookEdge = limit->getBuyOrSell() ? highestBuy : lowestSell;
    auto& tree = limit->getBuyOrSell() ? buyTree : sellTree;
    if (limit == bookEdge) {
        if (bookEdge != tree) {
            if (limit->getBuyOrSell() && bookEdge->getLeftChild() != NULL) {
                bookEdge = bookEdge->getLeftChild();
            }
            else if (!limit->getBuyOrSell() && bookEdge->getRightChild() != NULL) {
                bookEdge = bookEdge->getRightChild();
            }
            else {
                bookEdge = bookEdge->getParent();
            }
        }
        else {
            if (limit->getBuyOrSell() && bookEdge->getLeftChild() != NULL) {
                bookEdge = bookEdge->getLeftChild();
            }
            else if (!limit->getBuyOrSell() && bookEdge->getRightChild() != NULL) {
                bookEdge = bookEdge->getRightChild();
            }
            else {
                bookEdge = NULL;
            }
        }
    }
}

// update edge of stop book if current edge is empty
void Book::updateStopBookEdgeRemove(Limit* stoplevel) {
    auto& bookEdge = stoplevel->getBuyOrSell() ? lowestSell : highestStopSell;
    auto& tree = stoplevel->getBuyOrSell() ? stopBuyTree : stopSellTree;

    if (stoplevel == bookEdge) {
        if (bookEdge != tree) {
            if (stoplevel->getBuyOrSell() && bookEdge->getRightChild() != NULL) {
                bookEdge = bookEdge->getRightChild();
            }
            else if (!stoplevel->getBuyOrSell() && bookEdge->getLeftChild() != NULL) {
                bookEdge = bookEdge->getLeftChild();
            }
            else {
                bookEdge = bookEdge->getParent();
            }
        }
        else {
            if (stoplevel->getBuyOrSell() && bookEdge->getRightChild() != NULL) {
                bookEdge = bookEdge->getRightChild();
            }
            else if (!stoplevel->getBuyOrSell() && bookEdge->getLeftChild() != NULL) {
                bookEdge = bookEdge->getLeftChild();
            }
            else {
                bookEdge = NULL;
            }
        }
    }
}

// change avl's root limit if deleted
void Book::changeBookRoots(Limit* limit) {
    auto& tree = limit->getBuyOrSell() ? buyTree : sellTree;
    if (limit == tree) {
        if (limit->getRightChild() != NULL) {
            tree = tree->getRightChild();
            while (tree->getLeftChild() != NULL) {
                tree = tree->getLeftChild();
            }
        }
        else {
            tree = limit->getLeftChild();
        }
    }
}

// change root stop lvl if deleted, in avl
void Book::changeStopBookRoots(Limit* stopLevel) {
    auto& tree = stopLevel->getBuyOrSell() ? stopBuyTree : stopSellTree;
    if (stopLevel == tree) {
        if (stopLevel->getRightChild() != NULL) {
            tree = tree->getRightChild();
            while (tree->getLeftChild() != NULL) {
                tree = tree->getLeftChild();
            }
        }
        else {
            tree = stopLevel->getLeftChild();
        }
    }
}

// delete limit when empty
void Book::deleteLimit(Limit* limit) {
    updateBookEdgeRemove(limit);
    deleteFromLimitMap(limit->getLimitPrice(), limit->getBuyOrSell());
    changeBookRoots(limit);

    Limit* parent = limit->getParent();
    int limitPrice = limit->getLimitPrice();
    delete limit;

    while (parent != NULL) {
        parent = balance(parent);
        if (parent->getParent() != NULL) {
            if (parent->getParent()->getLimitPrice() > limitPrice) {
                parent->getParent()->setLeftChild(parent);
            }
            else {
                parent->getParent()->setRightChild(parent);
            }
        }
        parent = parent->getParent();
    }
}

// delete stop lvl after empty
void Book::deleteStopLevel(Limit* stopLevel) {
    updateStopBookEdgeRemove(stopLevel);
    deleteFromStopMap(stopLevel->getLimitPrice());
    changeStopBookRoots(stopLevel);

    Limit* parent = stopLevel->getParent();
    int stopPrice = stopLevel->getLimitPrice();
    delete stopLevel;
    while (parent != NULL) {
        parent = balanceStop(parent);
        if (parent->getParent() != NULL) {
            if (parent->getParent()->getLimitPrice() > stopPrice) {
                parent->getParent()->setLeftChild(parent);
            }
            else {
                parent->getParent()->setRightChild(parent);
            }
        }
        parent = parent->getParent();
    }
}

void Book::deleteFromOrderMap(int orderId) {
    orderMap.erase(orderId);
}

void Book::deleteFromLimitMap(int limitPrice, bool buyOrSell) {
    auto& limitMap = buyOrSell ? limitBuyMap : limitSellMap;
    limitMap.erase(limitPrice);
}

void Book::deleteFromStopMap(int stopPrice) {
    stopMap.erase(stopPrice);
}

// execute limit order as market order when (merge intervals) happens with highest buy or lowest sell
int Book::limitOrderAsMarketOrder(int orderId, bool buyOrSell, int shares, int limitPrice) {
    if (buyOrSell) {
        while (lowestSell != NULL && shares != 0 && lowestSell->getLimitPrice() <= limitPrice) {
            if (shares <= lowestSell->getTotalVolume()) {
                marketOrderHelper(orderId, buyOrSell, shares);
                return 0;
            }
            else {
                shares -= lowestSell->getTotalVolume();
                marketOrderHelper(orderId, buyOrSell, lowestSell->getTotalVolume());
            }
        }
        return shares;
    }
    else {
        while (highestBuy != NULL && shares != 0 && highestBuy->getLimitPrice() >= limitPrice) {
            if (shares <= highestBuy->getTotalVolume()) {
                marketOrderHelper(orderId, buyOrSell, shares);
                return 0;
            }
            else {
                shares -= highestBuy->getTotalVolume();
                marketOrderHelper(orderId, buyOrSell, highestBuy->getTotalVolume());
            }
        }
        return shares;
    }
}

// when a stlop limit order overlaps with highest buy or lowest sell, execute as market order
int Book::existingOrderAsMarketOrder(Order* headOrder, bool buyOrSell) {
    int shares = headOrder->getShares();
    int orderId = headOrder->getOrderId();
    int limitPrice = headOrder->getLimit();

    if (buyOrSell) {
        while (lowestSell != NULL && lowestSell->getLimitPrice() <= limitPrice) {
            if (shares <= lowestSell->getTotalVolume()) {
                deleteFromOrderMap(orderId);
                delete headOrder;
                marketOrderHelper(orderId, buyOrSell, shares);
                return 0;
            }
            else {
                shares -= lowestSell->getTotalVolume();
                marketOrderHelper(orderId, buyOrSell, lowestSell->getTotalVolume());
            }
        }
        return shares;
    }
    else {
        while (highestBuy != NULL && highestBuy->getLimitPrice() >= limitPrice) {
            if (shares <= highestBuy->getTotalVolume()) {
                deleteFromOrderMap(orderId);
                delete headOrder;
                marketOrderHelper(orderId, buyOrSell, shares);
                return 0;
            }
            else {
                shares -= highestBuy->getTotalVolume();
                marketOrderHelper(orderId, buyOrSell, shares);
            }
        }
        return shares;
    }
}

// stop limit order as limit order
int Book::stopLimitOrderAsLimitOrder(int orderId, bool buyOrSell, int shares, int limitPrice, int stopPrice) {
    if (buyOrSell && lowestSell != NULL && stopPrice <= lowestSell->getLimitPrice()) {
        addLimitOrder(orderId, buyOrSell, shares, limitPrice);
        return 0;
    }
    else if (!buyOrSell && highestBuy != NULL && stopPrice >= highestBuy->getLimitPrice()) {
        addLimitOrder(orderId, false, shares, limitPrice);
        return 0;
    }
    return shares;
}

// execute remaining stop orders
void Book::executeStopOrders(bool buyOrSell) {
    if (buyOrSell) {
        // if empty book, and cant finish stop order, then remains. BUY stop market orders
        while (lowestStopBuy != NULL && (lowestSell == NULL || lowestStopBuy->getLimitPrice() <= lowestSell->getLimitPrice())) {
            Order* headOrder = lowestStopBuy->getHeadOrder();
            if (headOrder->getLimit() == 0) {
                int shares = headOrder->getShares();
                headOrder->execute();
                if (lowestStopBuy->getSize() == 0) {
                    deleteStopLevel(lowestStopBuy);
                }

                deleteFromOrderMap(headOrder->getOrderId());
                delete headOrder;
                marketOrderHelper(0, true, shares);
            }
            else {
                stopLimitOrderToLimitOrder(headOrder, buyOrSell);
            }
        }
    }
    else {
        // if empty book and cant complete stop order then remains. SELL stop market orders
        while (highestStopSell != NULL && (highestBuy == NULL or highestStopSell->getLimitPrice() >= highestBuy->getLimitPrice())) {

        }
        Order* headOrder = lowestStopBuy->getHeadOrder();
        if (headOrder->getLimit() == 0) {
            int shares = headOrder->getShares();
            headOrder->execute();
            if (lowestStopBuy->getSize() == 0) {
                deleteStopLevel(lowestStopBuy);
            }
            deleteFromOrderMap(headOrder->getOrderId());
            delete headOrder;
            marketOrderHelper(0, false, shares);
        }
        else {
            stopLimitOrderToLimitOrder(headOrder, buyOrSell);
        }
    }
}

// make stop limit order -> limit order
void Book::stopLimitOrderToLimitOrder(Order* headOrder, bool buyOrSell) {
    auto& bookEdge = buyOrSell ? lowestStopBuy : highestStopSell;
    headOrder->execute();
    if (bookEdge->getSize() == 0) {
        deleteStopLevel(bookEdge);
    }

    // execute instantly
    int shares = existingOrderAsMarketOrder(headOrder, buyOrSell);

    if (shares != 0) {
        headOrder->setShares(shares);
        auto& limitMap = buyOrSell ? limitBuyMap : limitSellMap;

        if (limitMap.find(headOrder->getLimit()) == limitMap.end()) {
            addLimit(headOrder->getLimit(), buyOrSell);
        }
        limitMap.at(headOrder->getLimit())->append(headOrder);
    }
}

// executing market orders. if empty book and cant complete order then remains
void Book::marketOrderHelper(int orderId, bool buyOrSell, int shares) {
    auto & bookEdge = buyOrSell ? lowestSell : highestBuy;

    while (bookEdge != NULL && bookEdge->getHeadOrder()->getShares() <= shares) {
        Order* headOrder = bookEdge->getHeadOrder();
        shares -= headOrder->getShares();
        headOrder->execute();
        if (bookEdge->getSize() == 0) {
            deleteLimit(bookEdge);
        }
        deleteFromOrderMap(headOrder->getOrderId());
        delete headOrder;
        executedOrdersCount++;
    }

    if (bookEdge != NULL && shares != 0) {
        bookEdge->getHeadOrder()->partiallyFillOrder(shares);
        executedOrdersCount++;
    }
}

int Book::limitHeightDifference(Limit* limit) {
    int l = getLimitHeight(limit->getLeftChild());
    int r = getLimitHeight(limit->getRightChild());
    return l - r;
}

// rotations will define later
