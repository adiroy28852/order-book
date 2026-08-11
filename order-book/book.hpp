#pragma once
#include <cstddef>
#include <unordered_map>
#include <vector>
#include <random>
#include <unordered_set>

class Limit;
class Order;

class Book {
private:
    Limit *buyTree;
    Limit *sellTree;
    Limit *lowestSell;
    Limit *highestBuy;

    Limit *stopBuyTree;
    Limit *stopSellTree;
    Limit *highestStopSell;
    Limit *lowestStopBuy;

    std::unordered_map<int, Order*> orderMap;
    std::unordered_map<int, Limit*> limitBuyMap;
    std::unordered_map<int, Limit*> limitSellMap;
    std::unordered_map<int, Limit*> stopMap;

    void addLimit(int limitPrice, bool buyOrSell);
    void addStop(int stopPrice, bool buyOrSell);
    Limit* insert(Limit* root, Limit* limit, Limit* parent = nullptr);
    Limit* insertStop(Limit* root, Limit* limit, Limit* parent = nullptr);
    void updateBookEdgeInsert(Limit* newLimit);
    void updateStopBookEdgeInsert(Limit* newStop);
    void updateBookEdgeRemove(Limit* limit);
    void updateStopBookEdgeRemove(Limit* limit);
    void changeBookRoots(Limit* limit);
    void changeStopBookRoots(Limit* limit);
    void deleteLimit(Limit* limit);
    void deleteStopLevel(Limit* limit);
    void deleteFromOrderMap(int orderId);
    void deleteFromLimitMap(int limitPrice, bool buyOrSell);
    void deleteFromStopMap(int stopPrice);
    int limitOrderAsMarketOrder(int orderId, bool buyOrSell, int shares, int limitPrice);
    int stopOrderAsMarketOrder(int orderId, bool buyOrSell, int shares, int stopPrice);
    int existingOrderAsMarketOrder(Order* headOrder, bool buyOrSell);
    int stopLimitOrderAsLimitOrder(int orderId, bool buyOrSell, int shares, int limitPrice, int stopPrice);
    void executeStopOrders(bool buyOrSell);
    void stopLimitOrderToLimitOrder(Order* headOrder, bool buyOrSell);
    void marketOrderHelper(int orderId, bool buyOrSell, int shares);

public:
    Book (Limit *_buyTree, Limit *_sellTree, Limit *_lowestSell, Limit *_highestBuy
    , Limit *_stopBuyTree, Limit *_stopSellTree, Limit *_highestStopSell, Limit *_lowestStopBuy) 
        : buyTree(_buyTree), sellTree(_sellTree), lowestSell(_lowestSell), highestBuy(_highestBuy), 
        stopBuyTree(_stopBuyTree), stopSellTree(_stopSellTree), highestStopSell(_highestStopSell), lowestStopBuy(_lowestStopBuy) {}
    
    int executeOrdersCount = 0;
    int AVLTreeBalanceCount = 0;
};