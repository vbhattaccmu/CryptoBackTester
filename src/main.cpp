/**
 *  @file   backtester.cpp
 *  @brief  A simple backtester with Order Management and Matching Engine in C++.
 *  @author Vikram Bhattacharjee
 *  @date  2022-11-26
 ***********************************************/

#include <iostream>
#include <chrono>
#include "oms.hpp"
#include "types.hpp"
using namespace std;

int main()
{
    std::cout << "Hello Trader!\n";
    // Create strategy
    Strategy* strategy = new Strategy("MarketData", 0);
    // Create OMS Service object
    OMS* oms_ = new OMS(strategy);
    // Start processes 1-3
    oms_->invokeOMS();
    // Create an order
    Order order(0, 0, 0, 1, TradeType::Buy);
    // Send order 
    bool status = strategy->sendOrder(order, oms_->orderBook);
    // Start order matcher 
    oms_->orderMatcher();

    return EXIT_SUCCESS;
}
