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
#include "reader.hpp"

typedef int Size;
typedef int DeviceIdx;
typedef int Price;

const Size MAX_ORDER_SIZE = 10000;
const Size MAX_INSTRUMENT_SIZE = 50;

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
    Order(DeviceIdx _index, Price _price, Size _size, int64_t _orderID, TradeType _type) :
        index{ _index }, price{ _price }, size{ _size }, type{ _type }, orderID(_orderID) {}
    ~Order() = default;

    DeviceIdx index;
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
    DeviceIdx index;
    Price price;
    Size size;
};

/*
* MarketStatus struct
*/
struct MarketStatus {
    Size quantity;
    bool isLeft;
    std::unordered_map<DeviceIdx, std::pair<Size, bool>> map;

    MarketStatus() {
        quantity = 0;
        isLeft = false;
    }
    void updateState(DeviceIdx _idx, bool _isLeft) {
        if (map.find(_idx) != map.end()) {
            // todo
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