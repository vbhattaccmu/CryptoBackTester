/**
 *  @file   manager.hpp
 *  @brief  Manager Service.
 *  @author Vikram Bhattacharjee
 *  @date  2022-11-26
 ***********************************************/

#ifndef _MANAGER__HPP__
#define _MANAGER__HPP__

#include <chrono>
#include <thread>
#include <iostream>
#include <mutex>
#include <unordered_map>
#include <set>
#include <queue>
#include "types.hpp"
#include "reader.hpp"
#include "strategy.hpp"

using namespace std::chrono;

/*
 * Manager class
 * friend class: Strategy
*/
class Manager {
protected:
	Manager() {
		for (std::size_t i = 0; i < MAX_INSTRUMENT_SIZE; i++) {
			marketIdx.insert(i);
		}
	}
	~Manager() {}
private:
	bool invokeManager() {
		auto fileHandle = strategy->start();
		orderBook.clear();
		int64_t timestamp = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
		std::thread readerThread(triggerReader, fileHandle, timestamp);
		std::thread ManagerMatcher(orderMatcher);

     	readerThread.join();
		ManagerMatcher.join();

		// exception handling 
		for (const auto& exception : exceptions) {
			try {
				if (exception) {
					std::rethrow_exception(exception);
				}
			}
			catch (const std::exception& e) {
				std::cout << "exception occured" << e.what() << std::endl;
			}
		}
		return true;
	}

	void triggerReader(std::istream* file, int64_t timestamp) {
        for (DataReaderIterator loop(&file); loop != DataReaderIterator(); ++loop)
        {
            try {
				DeviceIdx index = stoi((*loop)[1]);
				marketIdx.insert(index);
			}
            catch (const exception& e) {
				std::lock_guard<std::mutex> lk(exceptionMutex);
				exceptions.push_back(e);
            }
        }
        
	}

    bool orderMatcher() {
		bool success = true;
		while (true) {
			std::unique_lock<std::mutex> lk(orderSynchroniationMutex);
			orderSyncCond.wait(lk, []() { return nextOrder < orderCount; });
			while (nextOrder < orderCount) {
				try {
					bool status = matchingHelper(orderBook[nextOrder]);
					if (status) {
						std::cout << "Successfully placed.." << orderBook[nextOrder];
					}
					else {
						std::cout << "Order not successful" << orderBook[nextOrder];
					}
					nextOrder.fetch_add(1);
				}
				catch (const exception& e) {
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

		if (not cs_map.empty() or marketdataStatus->isLeft) {
			if (marketdataStatus->isLeft) {
				qty = qty - marketdataStatus->quantity;
				if (qty >= 0) {
					marketdataStatus->isLeft = true;
					if (qty == 0) {
						return true;
					}
				}
				else {
					marketdataStatus->quantity = qty * (-1);
					marketdataStatus->isLeft = true;
					return true;
				}
			}
			else {
				while (qty > 0) {
					if (!cs_map.empty()) {
						qty = qty - marketdataStatus->quantity;
						if (qty >= 0) {
							marketdataStatus->isLeft = false;
							if (qty == 0) {
								return true;
							}
						}
						else {
							marketdataStatus->quantity = qty * (-1);
							return true;
						}
					}
					else {
						return false;
					}
				}
			}
		}
		else {
			// TODO
		}
		return false;
	}

public:
	
	Strategy* strategy;
	MarketStatus* marketdataStatus;

	// synchronization primitives
	std::atomic_flag dataReady;
	std::mutex orderSynchroniationMutex;
	std::mutex exceptionMutex;
	std::condition_variable orderSyncCond;
	std::atomic<unsigned long> orderCount;
	std::atomic<unsigned long> nextOrder;

	// list of exceptions occuring in Manager
	std::vector<std::exception_ptr> exceptions;
	// maintain OrderBook for Manager
	std::vector<Order> orderBook;
	// maintain number of instruments for Manager
	std::set<DeviceIdx> marketIdx;
};
#endif
