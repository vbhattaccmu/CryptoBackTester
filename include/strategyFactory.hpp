/**
 *  @file   strategyfactory.hpp
 *  @brief  Abstract Factory class for Strategy Class.
 *  @author Vikram Bhattacharjee
 *  @date  2022-11-26
 ***********************************************/

#ifndef _STRATEGY_FACTORY__HPP__
#define _STRATEGY_FACTORY__HPP__
#include "types.hpp"

/**
 * AbstractStratFactory provides an interface to the client
 * to define different strategies for the back testing application.
*/
class AbstractStratFactory {
public:
    AbstractStratFactory() {}

    virtual std::pair<std::istream*, std::string> start() = 0;
    virtual void stop() = 0;
    virtual bool sendOrder(Order& order, std::vector<Order>& orderBook) = 0;
    virtual bool fillOrder(Order& order, std::vector<MarketData>& marketDataMap) = 0;
    virtual void onMarketDataUpdate(Order& order) = 0;
    virtual void onPrint() = 0;

    virtual ~AbstractStratFactory() {}
};
#endif
