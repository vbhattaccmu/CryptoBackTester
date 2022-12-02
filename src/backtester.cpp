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
#include "strategyFactory.hpp"
using namespace std;

/**
 * Client side interface.
 */
void runBackTester(AbstractStratFactory* factory) {
    // Create OMS Service object
    OMS* oms_ = new OMS(factory);
    // Start processes 1-3
    oms_->invokeOMS();
    // Create an order
    Order order(0, 0, 0, 1, TradeType::Buy);
    // Send order 
    bool status = factory->sendOrder(order, oms_->orderBook);
    // Start order matcher 
    oms_->orderMatcher();   
}

void stopBackTester(AbstractStratFactory* factory) {
    factory->stop();
}

int main()
{
    std::cout << "Hello Trader!\n";
    
    AbstractStratFactory* factory = new Strategy("MarketData", 0);

    try {
        // run backtester
        runBackTester(factory);
    }
    catch (...) {
        // stop the service instead of error
        stopBackTester(factory);
    }

    delete factory;

    return EXIT_SUCCESS;
}
