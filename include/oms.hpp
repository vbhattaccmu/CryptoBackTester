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
	OMS(AbstractStratFactory* strategy) {
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
		// 0. preliminary setup
		static int64_t _timestamp;
		static Price _bidPrice;
		static Price _askPrice;
		static Size _bidSize;
		static Size _askSize;
		static bool _aggregator;

		// 1. get file handle from strategy
		auto strat_ = strategy_->start();
		auto fileHandle = strat_.first;

		// 2. Read MDR in a loop from the file handle
		for (MDRIterator loop(*fileHandle); loop != MDRIterator(); ++loop)
		{
			try {
				// save data to market maps.
				InstrumentIdx index = stoi((*loop)[SLOT1]);
				if (strat_.second == marketData) {

					_timestamp = stoi((*loop)[SLOT0]);
					_bidPrice = stoi((*loop)[SLOT2]);
					_askPrice = stoi((*loop)[SLOT3]);
					_bidSize = stoi((*loop)[SLOT4]);
					_askSize = stoi((*loop)[SLOT5]);
					MarketData data = MarketData(_timestamp, index, _bidPrice, _askPrice, _bidSize, _askSize);
					// update data to cache
					marketdataStatus->marketDataMap[index].push_back(data);
				}
				else {
					_timestamp = stoi((*loop)[SLOT0]);
					_bidPrice = stoi((*loop)[SLOT2]);
					_askPrice = stoi((*loop)[SLOT3]);
					_aggregator = stoi((*loop)[SLOT4]) > 0 ? true : false;
					PrintData data = PrintData(_timestamp, index, _bidPrice, _askPrice, _aggregator);

					marketdataStatus->printDataMap[index].push_back(data);

				}
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

	// this order matcher currently works with MarketData only.
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
		std::vector<MarketData> marketDataMap{};
		if (marketdataStatus->marketDataMap.find(order.index) != marketdataStatus->marketDataMap.end()) {
			marketDataMap = marketdataStatus->marketDataMap[order.index];
		}

		if (!marketDataMap.empty() or marketdataStatus->isLeft) {
			if (marketdataStatus->isLeft) {
				qty = qty - marketdataStatus->quantity;
				if (qty >= 0) {
					marketdataStatus->isLeft = true;
					if (qty == 0) {
						// 3.iv. Fill order, query existing map to check if the order can be filled. 
						strategy_->fillOrder(order, marketDataMap);
						// 3.v Update current market state 
						marketdataStatus->updateState(order.index, marketdataStatus->isLeft);

						return true;
					}
				}
				else {
					marketdataStatus->quantity = qty * (-1);
					marketdataStatus->isLeft = true;
					// 3.iv. Fill order, query existing map to check if the order can be filled. 
					strategy_->fillOrder(order, marketDataMap);
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
								// 3.iv. Fill order, query existing map to check if the order can be filled. 
								strategy_->fillOrder(order, marketDataMap);
								// 3.v Update current market state 
								marketdataStatus->updateState(order.index, marketdataStatus->isLeft);
								return true;
							}
						}
						else {
							marketdataStatus->quantity = qty * (-1);
							// 3.iv. Fill order, query existing map to check if the order can be filled. 
							strategy_->fillOrder(order, marketDataMap);
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
	AbstractStratFactory* strategy_;

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