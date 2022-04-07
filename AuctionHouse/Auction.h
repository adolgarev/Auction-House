#pragma once

#include <string>
#include <map>
#include "Db.h"
#include "Order.h"
#include "Timer.h"
#include "EventListener.h"
#include "InMemoryDb.h"

namespace auction {

	const std::string fundsItemName{ "funds" };

	class Auction : public EventListener<Order>
	{
		std::map<uint32_t, Order> idToOrder;
		uint32_t lastOrderId;
		InMemoryDb onHold;
		Db& db;
		int sellingFee;
		Timer<Order>& timer;
		int orderExpiresInSeconds;
		std::map<std::string, EventListener<Order>*> clientLiteners;
	public:
		Auction(Db& db, int sellingFee, Timer<Order>& timer, int orderExpiresInSeconds) : db(db),
			lastOrderId(0), sellingFee(sellingFee), timer(timer), orderExpiresInSeconds(orderExpiresInSeconds), onHold("onHold") {}
		int deposit(const std::string& login, const std::string& itemName, const int& amount);
		int withdraw(const std::string& login, const std::string& itemName, const int& amount);
		void sell(const std::string& login, const std::string& itemName, const int& amount, const int& minPrice);
		void buy(const std::string& login, const uint32_t& orderId, const int& price);
		const std::map<uint32_t, Order>& orderBook();
		const std::map<std::string, int> inventory(std::string login);
		void on(const Order* order);
		void addClientListener(const std::string& login, EventListener<Order>* listener);
		void removeClientLitener(const std::string& login);
	};
}