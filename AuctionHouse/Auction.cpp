#include "Auction.h"

namespace auction {

	int Auction::deposit(std::string login, std::string itemName, int amount)
	{
		auto t = db.startTransaction();
		int newAmount = db.select(login, itemName) + amount;
		db.update(login, itemName, newAmount);
		t.commit();
		return newAmount;
	}

	int Auction::withdraw(std::string login, std::string itemName, int amount)
	{
		auto t = db.startTransaction();
		int newAmount = db.select(login, itemName) - amount;
		int itemsOnHold = onHold.select(login, itemName);
		if (newAmount - itemsOnHold < 0)
			throw "";
		db.update(login, itemName, newAmount);
		t.commit();
		return newAmount;
	}

	void Auction::sell(std::string login, std::string itemName, int amount, int minPrice)
	{
		auto t = db.startTransaction();

		int itemsOnHold = onHold.select(login, itemName);
		int curAvailableItems = db.select(login, itemName) - itemsOnHold;
		if (curAvailableItems - amount < 0)
			throw "";

		int fundsOnHold = onHold.select(login, fundsItemName);
		int curAvailableFunds = db.select(login, fundsItemName) - fundsOnHold;
		if (curAvailableFunds - sellingFee < 0)
			throw "";

		onHold.update(login, itemName, itemsOnHold + amount);
		onHold.update(login, fundsItemName, fundsOnHold + sellingFee);

		++lastOrderId;
		Order order{ lastOrderId, login, itemName, amount, minPrice, "" };
		idToOrder[lastOrderId] = order;
		timer.setTimeout(orderExpiresInSeconds, this, &idToOrder[lastOrderId]);
	}

	void Auction::buy(std::string login, uint32_t orderId, int price)
	{
		auto t = db.startTransaction();

		auto it = idToOrder.find(orderId);
		if (it == idToOrder.end())
			throw "";

		if (it->second.price >= price)
			throw "";

		int fundsOnHold = onHold.select(login, fundsItemName);
		int curAvailableFunds = db.select(login, fundsItemName) - fundsOnHold;
		if (curAvailableFunds - price < 0)
			throw "";

		it->second.price = price;
		it->second.loginTo = login;
	}

	const std::map<uint32_t, Order>& Auction::orderBook()
	{
		return idToOrder;
	}

	std::map<std::string, int> Auction::inventory(std::string login)
	{
		auto itemNameToAmount{ db.select(login) };
		for (auto& elt : itemNameToAmount)
		{
			elt.second -= onHold.select(login, elt.first);
		}
		return itemNameToAmount;
	}

	void Auction::on(Order* order)
	{
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
			db.update(order->loginTo, order->itemName, availableFunds - order->price);
		}
		else
		{
			db.update(order->loginFrom, fundsItemName, availableFunds - sellingFee);
		}

		idToOrder.erase(order->id);

		t.commit();
	}
}