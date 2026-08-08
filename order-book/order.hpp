#pragma once
#include <bits/stdc++.h>

class Order {
private:
    int id;
    bool buySell;
    int qtty;
    int limit;
    Order* next;
    Order* prev;
    Limit* parent;

    friend class Limit;

public:
    Order(int _id, bool _buySell, int _qtty, int _limit);

    // getters
    int getShares const();
    int getOrderId const();
    bool getBuySell const();
    int getLimit const();
    Limit* getParentLimit const();

    void partialFillOrder(int shares);
    void cancel();
    void execute();
    void modifyOrder(int newShares, int newLimit);
    void setShares(int shares);


    void print() const;
};