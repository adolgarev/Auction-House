#include <stdexcept>

#include "easylogging++.h"
#include "Auction.h"
#include "InsufficientItemsEception.h"

namespace auction {

	int Auction::deposit(const std::string& login, const std::string& itemName, const int& amount)
	{
		if (login == "")
			throw std::invalid_argument("Login may not be empty");

		if (itemName == "")
			throw std::invalid_argument("ItemName may not be empty");

		if (amount <= 0)
			throw std::invalid_argument("Amount must be a positive integer");

		LOG(DEBUG) << "Deposit" << login << itemName << amount;
		auto t = db.startTransaction();
		int newAmount = db.select(login, itemName) + amount;
		db.update(login, itemName, newAmount);
		t.commit();
		return newAmount;
	}

	int Auction::withdraw(const std::string& login, const std::string& itemName, const int& amount)
	{
		if (login == "")
			throw std::invalid_argument("Login may not be empty");

		if (itemName == "")
			throw std::invalid_argument("ItemName may not be empty");

		if (amount <= 0)
			throw std::invalid_argument("Amount must be a positive integer");

		LOG(DEBUG) << "Withdraw" << login << itemName << amount;
		auto t = db.startTransaction();
		int itemsOnHold = onHold.select(login, itemName);
		int curAvailableItems = db.select(login, itemName) - itemsOnHold;
		if (curAvailableItems - amount < 0)
			throw InsufficientItemsException(itemName, curAvailableItems, amount);
		int newAmount = curAvailableItems + itemsOnHold - amount;
		db.update(login, itemName, newAmount);
		t.commit();
		return newAmount;
	}

	void Auction::sell(const std::string& login, const std::string& itemName, const int& amount, const int& minPrice)
	{
		if (login == "")
			throw std::invalid_argument("Login may not be empty");

		if (itemName == "")
			throw std::invalid_argument("ItemName may not be empty");

		if (amount <= 0)
			throw std::invalid_argument("Amount must be a positive integer");

		if (minPrice <= 0)
			throw std::invalid_argument("MinPrice must be a positive integer");

		LOG(DEBUG) << "Sell" << login << itemName << amount << minPrice;
		auto t = db.startTransaction();

		int itemsOnHold = onHold.select(login, itemName);
		int curAvailableItems = db.select(login, itemName) - itemsOnHold;
		if (curAvailableItems - amount < 0)
			throw InsufficientItemsException(itemName, curAvailableItems, amount);

		int fundsOnHold = onHold.select(login, fundsItemName);
		int curAvailableFunds = db.select(login, fundsItemName) - fundsOnHold;
		if (curAvailableFunds - sellingFee < 0)
			throw InsufficientItemsException(fundsItemName, curAvailableFunds, sellingFee);

		onHold.update(login, itemName, itemsOnHold + amount);
		onHold.update(login, fundsItemName, fundsOnHold + sellingFee);

		++lastOrderId;
		Order order{ lastOrderId, login, itemName, amount, minPrice, "" };
		idToOrder[lastOrderId] = order;
		timer.setTimeout(orderExpiresInSeconds, this, &idToOrder[lastOrderId]);
	}

	void Auction::buy(const std::string& login, const uint32_t& orderId, const int& price)
	{
		if (login == "")
			throw std::invalid_argument("Login may not be empty");

		if (price <= 0)
			throw std::invalid_argument("Price must be a positive integer");

		LOG(DEBUG) << "Buy" << login << orderId << price;
		auto t = db.startTransaction();

		auto it = idToOrder.find(orderId);
		if (it == idToOrder.end())
			throw std::invalid_argument("No such order");

		if (it->second.loginFrom == login)
			throw std::invalid_argument("You can't buy from yourself");

		if (it->second.price > price && it->second.loginTo == "")
			throw std::invalid_argument("Price is too low");

		if (it->second.price == price && it->second.loginTo != "")
			throw std::invalid_argument("Price is too low");

		int fundsOnHold = onHold.select(login, fundsItemName);
		int curAvailableFunds = db.select(login, fundsItemName) - fundsOnHold;
		if (curAvailableFunds - price < 0)
			throw InsufficientItemsException(fundsItemName, curAvailableFunds, price);

		onHold.update(login, fundsItemName, fundsOnHold + price);

		if (it->second.loginTo != "")
		{
			int fundsOnHold = onHold.select(it->second.loginTo, fundsItemName);
			onHold.update(it->second.loginTo, fundsItemName, fundsOnHold - it->second.price);
		}

		it->second.price = price;
		it->second.loginTo = login;
	}

	const std::map<uint32_t, Order>& Auction::orderBook()
	{
		LOG(DEBUG) << "OrderBook";
		return idToOrder;
	}

	const std::map<std::string, int> Auction::inventory(std::string login)
	{
		if (login == "")
			throw std::invalid_argument("Login may not be empty");

		LOG(DEBUG) << "Inventory" << login;
		auto itemNameToAmount{ db.select(login) };
		for (auto& elt : itemNameToAmount)
		{
			elt.second -= onHold.select(login, elt.first);
		}
		return itemNameToAmount;
	}

	void Auction::on(const Order* order)
	{
		LOG(DEBUG) << "Order expired" << order->id << order->loginFrom << order->itemName
			<< order->amount << order->price << order->loginTo;
		auto t = db.startTransaction();

		int itemsOnHold = onHold.select(order->loginFrom, order->itemName);
		onHold.update(order->loginFrom, order->itemName, itemsOnHold - order->amount);
		if (order->loginTo != "")
		{
			int availableItems = db.select(order->loginFrom, order->itemName);
			db.update(order->loginFrom, order->itemName, availableItems - order->amount);
			availableItems = db.select(order->loginTo, order->itemName);
			db.update(order->loginTo, order->itemName, availableItems + order->amount);
		}

		int fundsOnHold = onHold.select(order->loginFrom, fundsItemName);
		onHold.update(order->loginFrom, fundsItemName, fundsOnHold - sellingFee);
		int availableFunds = db.select(order->loginFrom, fundsItemName);
		if (order->loginTo != "")
		{
			db.update(order->loginFrom, fundsItemName, availableFunds + order->price - sellingFee);
			fundsOnHold = onHold.select(order->loginTo, fundsItemName);
			onHold.update(order->loginTo, fundsItemName, fundsOnHold - order->price);
			availableFunds = db.select(order->loginTo, fundsItemName);
			db.update(order->loginTo, fundsItemName, availableFunds - order->price);
		}
		else
		{
			db.update(order->loginFrom, fundsItemName, availableFunds - sellingFee);
		}


		auto it = clientLiteners.find(order->loginFrom);
		if (it != clientLiteners.end())
			it->second->on(order);
		if (order->loginTo != "")
		{
			it = clientLiteners.find(order->loginTo);
			if (it != clientLiteners.end())
				it->second->on(order);
		}

		idToOrder.erase(order->id);

		t.commit();
	}

	void Auction::addClientListener(const std::string& login, EventListener<Order>* listener)
	{
		if (login == "")
			throw std::invalid_argument("Login may not be empty");

		clientLiteners[login] = listener;
	}

	void Auction::removeClientLitener(const std::string& login)
	{
		if (login == "")
			throw std::invalid_argument("Login may not be empty");

		clientLiteners.erase(login);
	}
}