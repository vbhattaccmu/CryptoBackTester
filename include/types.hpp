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
const Size MAX_INSTRUMENT_SIZE = 50;
const std::string marketData = "MarketData";
const std::string printData = "printData";

#define SLOT0 0
#define SLOT1 1
#define SLOT2 2
#define SLOT3 3
#define SLOT4 4
#define SLOT5 4

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
* Attributes: bidPrice, askPrice, bidSize, askSize
*/
#pragma pack (4)
struct MarketData {
    int64_t timestamp;
    InstrumentIdx index;
    Price bidPrice;
    Price askPrice;
    Size bidSize;
    Size askSize;

    MarketData(int64_t _timestamp, InstrumentIdx _index, Price _bidPrice, Price _askPrice, Size _bidSize, Size _askSize) :
        timestamp{ _timestamp }, index {_index}, bidPrice{ _bidPrice }, askPrice{ _bidPrice }, bidSize{ _bidSize }, askSize{ _askSize } {}
    ~MarketData() = default;

};

/*
* MarketData struct
* Attributes: bidPrice, askPrice, bidSize, askSize
*/
#pragma pack (4)
struct PrintData {
    int64_t timestamp;
    InstrumentIdx index;
    Price bidPrice;
    Price askPrice;
    bool aggregator;

    PrintData(int64_t _timestamp, InstrumentIdx _index, Price _bidPrice, Price _askPrice, bool _aggregator) :
        timestamp{ _timestamp }, index{ _index }, bidPrice{ _bidPrice }, askPrice{ _bidPrice }, aggregator{ _aggregator } {}
    ~PrintData() = default;

};


/*
* MarketState struct
* Total volume present per Instrument Idx.
*/
struct MarketStatus {
    Size quantity;// total volume 
    bool isLeft;
    // InstrumentMap maintains total volume available per instrument
    std::unordered_map<InstrumentIdx, std::pair<Size, bool>> map;
    // marketDataMap maintains current marketData State 
    std::unordered_map<InstrumentIdx, std::vector<MarketData>> marketDataMap;
    // printData maintains current printData State
    std::unordered_map<InstrumentIdx, std::vector<PrintData>> printDataMap;

    MarketStatus() {
        quantity = 0;
        isLeft = false;
    }
    void updateState(InstrumentIdx _idx, bool _isLeft) {
        if (map.find(_idx) == map.end()) {
            map[_idx].first = _idx;
            map[_idx].second = _isLeft;
        }
        else {
            if (!_isLeft) {
                marketDataMap[_idx].clear();
                printDataMap[_idx].clear();
            }
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