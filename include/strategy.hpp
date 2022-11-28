/**
 *  @file   strategy.hpp
 *  @brief  Core strategy module.
 *  @author Vikram Bhattacharjee
 *  @date  2022-11-26
 ***********************************************/

#include <vector>
#include "types.hpp"
#include "strategyFactory.hpp"
#include "manager.hpp"

#ifndef _STRATEGY__HPP__
#define _SRATEGY__HPP__

class Strategy: public AbstractStratFactory {
public:	
	Strategy(std::string marketData, DeviceIdx idx) {
		listIdx.reserve(MAX_INSTRUMENT_SIZE);
		data = marketData;
		listIdx.emplace_back(idx);
	}

	std::istream* start() {
		if (data == "Options") {
			rfile.open("DATA_TYPE1.csv");
		}
		else {
			rfile.open("DATA_TYPE2.csv");
		}
		return &rfile;
	}

	bool processOrder(Order& order) {
		std::cout << "Order received..sending to Management Service" << std::endl;
		// update order to order book
		manager->orderBook.emplace_back(order);
		// increment order count by 1
		manager->orderCount.fetch_add(1);
		// synchronize all threads listening to manager
		manager->orderSyncCond.notify_one();
		std::cout << "Order sent successfully ." << std::endl;

		return true;
	}

	void halt() {
		std::cout << "Starting cleanup.." << std::endl;
		listIdx.clear();
		std::cout << "Cleanup complete.." << std::endl;
	}

	~Strategy() {
		listIdx.clear();
	}

private:
	Portfolio portflio;
	std::ifstream rfile;
	std::vector<DeviceIdx> listIdx;
	std::string data;
	Manager* manager;
};
#endif