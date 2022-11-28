/**
 *  @file   strategyfactory.hpp
 *  @brief  Abstract Factory class for Strategy Class.
 *  @author Vikram Bhattacharjee
 *  @date  2022-11-26
 ***********************************************/

#ifndef _STRATEGY_FACTORY__HPP__
#define _STRATEGY_FACTORY__HPP__

/**
 * AbstractStratFactory provides an interface to the client
 * to define different strategies for the back testing application.
*/
class AbstractStratFactory {
public:
    AbstractStratFactory() {}

    virtual std::istream* start() = 0;
    virtual bool processOrder() = 0;
    virtual void halt() = 0;

    virtual ~AbstractStratFactory() {}
};
#endif