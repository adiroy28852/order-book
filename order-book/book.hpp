#pragma once
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

public:
    Book (Limit *_buyTree, Limit *_sellTree, Limit *_lowestSell, Limit *_highestBuy
    , Limit *_stopBuyTree, Limit *_stopSellTree, Limit *_highestStopSell, Limit *_lowestStopBuy) 
        : buyTree(_buyTree), sellTree(_sellTree), lowestSell(_lowestSell), highestBuy(_highestBuy), 
        stopBuyTree(_stopBuyTree), stopSellTree(_stopSellTree), highestStopSell(_highestStopSell), lowestStopBuy(_lowestStopBuy) {}
};