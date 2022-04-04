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
	public:
		Auction(Db& db, int sellingFee, Timer<Order>& timer, int orderExpiresInSeconds) : db(db),
			lastOrderId(0), sellingFee(sellingFee), timer(timer), orderExpiresInSeconds(orderExpiresInSeconds), onHold("onHold") {}
		int deposit(std::string login, std::string itemName, int amount);
		int withdraw(std::string login, std::string itemName, int amount);
		void sell(std::string login, std::string itemName, int amount, int minPrice);
		void buy(std::string login, uint32_t orderId, int price);
		const std::map<uint32_t, Order>& orderBook();
		std::map<std::string, int> inventory(std::string login);
		void on(Order* order);
	};
}