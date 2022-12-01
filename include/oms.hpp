/**
 *  @file   OMS.hpp
 *  @brief  OMS Service.
 *  @author Vikram Bhattacharjee
 *  @date  2022-11-26
 ***********************************************/

#ifndef _OMS__HPP__
#define _OMS__HPP__

#include <chrono>
#include <thread>
#include <iostream>
#include <mutex>
#include <unordered_map>
#include <set>
#include <queue>
#include <future>
#include "mdr.hpp"
#include "strategy.hpp"
#include "types.hpp"

using namespace std::chrono;

/*
 * OMS class
 * friend class: Strategy
*/
class OMS {
public:
	OMS(Strategy* strategy) {
		for (std::size_t i = 0; i < MAX_INSTRUMENT_SIZE; i++) {
			marketIdx.insert(i);
		}
		strategy_ = strategy;
		marketdataStatus = new MarketStatus();
	}
	~OMS() {
		exceptions.clear();
		orderBook.clear();
		delete marketdataStatus;
		delete strategy_;
	}

	bool invokeOMS() {
		orderBook.clear();
		// 2. read MarketData/PrintData
		std::future<void> readerFuture = std::async(std::launch::deferred, &OMS::triggerReader, this);
		// 3 (i-ii) are covered from strategy and from main.cpp
		// Rest are covered as a part of order matcher
		orderMatcher();

		// general exception handling 
		for (const auto& exception : exceptions) {
			try {
				std::cout << "exception occured" << exception.what() << std::endl;
			}
			catch (const std::exception& e) {
				std::cout << "exception occured" << e.what() << std::endl;
			}
		}
		return true;
	}

	void triggerReader() {
		// 1. get file handle from strategy
		auto fileHandle = strategy_->start();
		int64_t timestamp = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

		// 2. Read MDR in a loop from the file handle
        for (MDRIterator loop(*fileHandle); loop != MDRIterator(); ++loop)
        {
            try {
				InstrumentIdx index = stoi((*loop)[1]);
				marketIdx.insert(index);
			}
            catch (const std::exception& e) {
				std::lock_guard<std::mutex> lk(exceptionMutex);
				exceptions.push_back(e);
            }
        }
		// at the end of the day stop the strategy and print portfolio.
		strategy_->stop();
	}

    bool orderMatcher() {
		bool success = true;
		while (true) {
			std::unique_lock<std::mutex> lk(orderSynchroniationMutex);
			while (nextOrder < orderCount) {
				try {
					bool status = matchingHelper(orderBook[nextOrder]);
					if (status) {
						std::cout << "Successfully placed.." << orderBook[nextOrder].orderID << std::endl;
					}
					else {
						std::cout << "Order not successful" << orderBook[nextOrder].orderID << std::endl;
					}
					nextOrder.fetch_add(1);
				}
				catch (const std::exception& e) {
					std::lock_guard<std::mutex> lk(exceptionMutex);
					exceptions.push_back(e);
					success = false;
				}
			}
		}
		std::cout << "Mathing Process Ended" << std::endl;
		return (success ? true : false);
    }

	bool matchingHelper(Order& order) {
		auto& cs_map = marketdataStatus->map[order.index];
		long qty = marketdataStatus->quantity;

		if (!marketdataStatus->map.empty() or marketdataStatus->isLeft) {
			if (marketdataStatus->isLeft) {
				qty = qty - marketdataStatus->quantity;
				if (qty >= 0) {
					marketdataStatus->isLeft = true;
					if (qty == 0) {
						// 3.iv. Fill order 
						strategy_->fillOrder(order);
						// 3.v Update current market state 
						marketdataStatus->updateState(order.index, marketdataStatus->isLeft);
						return true;
					}
				}
				else {
					marketdataStatus->quantity = qty * (-1);
					marketdataStatus->isLeft = true;
					// 3.iv. Fill order 
					strategy_->fillOrder(order);
					// 3.v Update current market state 
					marketdataStatus->updateState(order.index, marketdataStatus->isLeft);
					return true;
				}
			}
			else {
				while (qty > 0) {
					if (!marketdataStatus->map.empty()) {
						qty = qty - marketdataStatus->quantity;
						if (qty >= 0) {
							marketdataStatus->isLeft = false;
							if (qty == 0) {
								// 3.iv. Fill order 
								strategy_->fillOrder(order);
								// 3.v Update current market state 
								marketdataStatus->updateState(order.index, marketdataStatus->isLeft);
								return true;
							}
						}
						else {
							marketdataStatus->quantity = qty * (-1);
							// 3.iv. Fill order 
							strategy_->fillOrder(order);
							// 3.v Update current market state 
							marketdataStatus->updateState(order.index, marketdataStatus->isLeft);
							return true;
						}
					}
					else {
						return false;
					}
				}
			}
		}
		return false;
	}

public:
    // Market State is saved currently as    
	// std::unordered_map<InstrumentIdx, std::pair<Size, bool>> map;
	MarketStatus* marketdataStatus;
	Strategy* strategy_;

	// synchronization primitives
	std::mutex orderSynchroniationMutex;
	std::mutex exceptionMutex;
	std::atomic<unsigned long> orderCount;
	std::atomic<unsigned long> nextOrder;

	// list of exceptions occuring in OMS
	std::vector<std::exception> exceptions;
	// maintain OrderBook for OMS
	std::vector<Order> orderBook;
	// maintain number of instruments for OMS
	std::set<InstrumentIdx> marketIdx;
};
#endif