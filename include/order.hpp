/**
 *  @file   types.hpp
 *  @brief  General types module.
 *  @author Vikram Bhattacharjee
 *  @date  2022-11-26
 ***********************************************/

#ifndef _TYPES__HPP__
#define _TYPES__HPP__

#include <sstream>

# if __WORDSIZE == 64
typedef long int  int64_t;
# else
typedef long long int int64_t;
# endif

typedef std::size_t Size;
typedef int InstrumentIdx;
typedef long int Price;

#pragma pack (4)
struct MarketData {
    int64_t timestamp;
    InstrumentIdx index;
    Price bidPrice;
    Price askPrice;
    Size bidSize;
    Size askSize;
};

#pragma pack (4)
struct PrintData {
    int64_t timestamp;
    InstrumentIdx index;
    Price tradePrice;
    Size contractSize;
    bool aggressorSide;
};
#endif