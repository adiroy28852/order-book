#pragma once
#include <bits/stdc++.h>

class Order;

class Limit {
private:
    int limitPrice;
    int size;
    int totalVolume;
    bool buySell;
    Limit *parent;
    Limit *left;
    Limit *right;
    Order *headOrder;
    Order *tailOrder;
    friend class Order;

public:
    Limit(int _limitPrice, bool _buySell, int _size {0}, int _totalVolume {0});
    ~Limit();

    Order* getHeadOrder();
    int getLimitPrice() const;
    int getSize() const;
    int getTotalVolume() const;
    bool getBuySell() const;
    Limit* getParent() const;
    Limit* getLeftChidl() const;
    Limit* getRightChild() const;
    void setParent(Limit* _parent);
    void setLeftChild(Limit* _leftChild);
    void setRightChild(Limit* _rightChild);
    void partialFill(int shares);

    void append(Order *order);
    void getBack() const;
    void getFront() const;
    void print() const;
};