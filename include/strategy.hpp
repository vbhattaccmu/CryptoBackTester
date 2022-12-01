/**
 *  @file   strategy.hpp
 *  @brief  Core strategy module.
 *  @author Vikram Bhattacharjee
 *  @date  2022-11-26
 ***********************************************/

#ifndef _STRATEGY__HPP__
#define _SRATEGY__HPP__

#include <vector>
#include "types.hpp"
#include "strategyFactory.hpp"
#include "oms.hpp"

class Strategy: public AbstractStratFactory {
public:	
	Strategy(std::string marketData, InstrumentIdx idx) {
		listIdx.reserve(MAX_INSTRUMENT_SIZE);
		data = marketData;
		listIdx.emplace_back(idx);
	}

	std::istream* start() {
		if (data == "MarketData") {
			rfile.open("20220603.csv");
		}
		else {
			rfile.open("20220603_prints.csv");
		}
		return &rfile;
	}
	/*
	* sendOrder is an interface to submit orders from strategy to OMS.
	*/
	bool sendOrder(Order& order, std::vector<Order>& orderBook) {
		std::cout << "Order received..sending to Management Service" << std::endl;
		orderBook.emplace_back(order);
		std::cout << "Order sent successfully ." << std::endl;
		return true;
	}

	/*
	* fillOrder updates current portfolio 
	* updated if order is matched.
	*/
	bool fillOrder(Order& order) {
		portflio.orders.push_back(order);
		return true;
	}

	/*
	* onMarketDataUpdate is void for now
	*/
	void onMarketDataUpdate(Order& order) {
		return;
	}

	/*
	* onPrint is void for now
	*/
	void onPrint() {
		return;
	}

	/*
	* clear current strategy, print day' portfilio.
	* 
	*/
	void stop() {
		std::cout << "Your current portfolio.." << std::endl;
		std::cout << "Instrument" << "OrderID" << "Price" << "Size" << std::endl;
		for (auto order : portflio.orders) {
			order.print();
		}
		std::cout << "Starting cleanup.." << std::endl;
		listIdx.clear();
		portflio.orders.clear();
		delete &rfile;
		std::cout << "Cleanup complete.." << std::endl;
	}

	~Strategy() {
		listIdx.clear();
	}

private:
	Portfolio portflio;
	std::ifstream rfile;
	std::vector<InstrumentIdx> listIdx;
	std::string data;
};
#endif