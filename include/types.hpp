/**
 *  @file   types.hpp
 *  @brief  General types module.
 *  @author Vikram Bhattacharjee
 *  @date  2022-11-26
 ***********************************************/

#ifndef _TYPES__HPP__
#define _TYPES__HPP__

#include <sstream>
#include <vector>
#include "unordered_map"
#include "mdr.hpp"

typedef int Size;
typedef int InstrumentIdx;
typedef int Price;

const Size MAX_ORDER_SIZE = 10000;
const Size MAX_DEVICE_SIZE = 50;

/*
* TradeType enum (Buy/Sell)
*/
enum class TradeType { 
    Buy, 
    Sell 
};

/*
* Order class 
* Attributes: Instrument ID, price, size, type
*/
struct Order {
    Order() {}
    Order(InstrumentIdx _index, Price _price, Size _size, int64_t _orderID, TradeType _type) :
        index{ _index }, price{ _price }, size{ _size }, type{ _type }, orderID(_orderID) {}
    ~Order() = default;

    void print() {
        std::cout << this->index << this->orderID << this->price << this->size << std::endl;
    }

    InstrumentIdx index;
    int64_t orderID;
    Price price;
    Size size;
    TradeType type;
};


/*
* MarketData struct
* Attributes: timestamp, index, price, size
*/
#pragma pack (4)
struct MarketData {
    int64_t timestamp;
    InstrumentIdx index;
    Price price;
    Size size;
};

/*
* MarketState struct
* Attributes: bidPrice, askPrice, bidSize, askSize
*/
struct MarketStatus {
    Size quantity;
    bool isLeft;
    std::unordered_map<InstrumentIdx, std::pair<Size, bool>> map;

    MarketStatus() {
        quantity = 0;
        isLeft = false;
    }
    void updateState(InstrumentIdx _idx, bool _isLeft) {
        if (map.find(_idx) == map.end()) {
            map[_idx].first = _idx;
            map[_idx].second = _isLeft;
        }        
    }

};

/*
* Portfolio struct
* Attributes: a vector of orders
*/
struct Portfolio {
    std::vector<Order> orders;
    Portfolio() {
        orders.reserve(MAX_ORDER_SIZE);
    }
};
#endif