#include "limit.hpp"
#include "order.hpp"
#include <cstddef>
#include <iostream>
#include <ostream>
#include <type_traits>

#pragma once

Limit::Limit(int _limitPrice, bool _buyOrSell, int _size, int _totalVolume)
    : limitPrice(_limitPrice), buyOrSell(_buyOrSell), size(_size), totalVolume(_totalVolume),
    parent(nullptr), leftChild(nullptr), rightChild(nullptr), headOrder(nullptr), tailOrder(nullptr) {}

Limit::~Limit() {
    if (parent != nullptr) {
        bool leftOrRightChild = (limitPrice < parent->getLimitPrice());

        if (leftChild == nullptr) {
            if (leftOrRightChild) {
                parent->leftChild = rightChild;
            } 
            else {
                parent->rightChild = rightChild;
            }
            if (rightChild != nullptr) {
                rightChild->setParent(parent);
            }
            return;
        }
        else if (rightChild == nullptr) {
            if(leftOrRightChild) {
                parent->leftChild = leftChild;
            }
            else {
                parent->rightChild = leftChild;
            }

            leftChild->setParent(parent);
            return;
        }

        Limit *temp = rightChild;

        while (temp->getLeftChild() != nullptr) {
            temp = temp->getLeftChild();
        }

        if (rightChild->getLeftChild() != nullptr) {
            temp->getParent()->setLeftChild(temp->getRightChild());
            if (temp->getRightChild() != nullptr) {
                temp->getRightChild()->setParent(temp->getParent());
            }
            temp->setRightChild(rightChild);
            rightChild->setParent(temp);
        }

        temp->setParent(parent);
        temp->setLeftChild(leftChild);
        leftChild->setParent(temp);

        if (leftOrRightChild) {
            parent->leftChild = temp;
        }
        else {
            parent->rightChild = temp;
        }
    }
    else {
        if (leftChild == nullptr && rightChild == nullptr) {
            return;
        }
        else if (leftChild == nullptr) {
            rightChild->setParent(nullptr);
            return;
        }
        else if (rightChild == nullptr) {
            leftChild->setParent(nullptr);
            return;
        }

        Limit* temp = rightChild;
        while (temp->getLeftChild() != nullptr) {
            temp = temp->getLeftChild();
        }
        if (rightChild->getLeftChild() != nullptr) {
            temp->getParent()->setLeftChild(temp->getRightChild());
            if (temp->getRightChild() != nullptr) {
                temp->getRightChild()->setParent(temp->getParent());
            }
            temp->setRightChild(rightChild);
            rightChild->setParent(temp);
        }
        temp->setParent(parent);
        temp->setLeftChild(leftChild);
        leftChild->setParent(temp);
    }
}

Order* Limit::getHeadOrder() const {
    return headOrder;
}

int Limit::getLimitPrice() const {
    return limitPrice;
}

int Limit::getSize() const {
    return size;
}

int Limit::getTotalVolume() const {
    return totalVolume;
}

bool Limit::getBuyOrSell() const {
    return buyOrSell;
}

Limit* Limit::getLeftChild() const {
    return leftChild;
}

Limit* Limit::getRightChild() const {
    return rightChild;
}

void Limit::setParent(Limit* newParent) {
    parent = newParent;
}

void Limit::setLeftChild(Limit* newLeftChild) {
    leftChild = newLeftChild;
}

void Limit::setRightChild(Limit* newRightChild) {
    rightChild = newRightChild;
}

void Limit::partiallyFillTotalVolume(int orderedShares) {
    totalVolume -= orderedShares;
}

void Limit::append(Order *order) {
    if (headOrder == nullptr) {
        headOrder = tailOrder = order;
    }
    else {
        tailOrder->nextOrder = order;
        order->prevOrder = tailOrder;
        order->nextOrder = nullptr;
        tailOrder = order;
    }
    size += 1;
    totalVolume += order->getShares();
    order->parentLimit = this;
}

void Limit::printForward() const {
    Order* current = headOrder;
    while (current != nullptr) {
        std::cout << current->getOrderId() << " ";
        current = current->nextOrder;
    }
    std::cout << std::endl;
}

void Limit::printBackward() const {
    Order* current = tailOrder;
    while (current != nullptr) {
        std::cout<< current->getOrderId() << " ";
        current = current->prevOrder;
    }
    std::cout << std::endl;
}

void Limit::print() const {
    std::cout << "Limit Price: " << limitPrice
    << ", Limit Volume: " << totalVolume
    << ", Limit Size: " << size << std::endl;
}