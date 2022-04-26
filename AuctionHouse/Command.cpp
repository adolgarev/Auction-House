#include "Command.h"

namespace auction
{
	void DepositCommand::execute(Auction& auc, const std::string& login, const std::vector<std::string>& argv, std::ostream& out)
	{
		if (argv.size() != 3)
		{
			throw std::runtime_error("Invalid request");
		}
		std::string itemName = argv[1];
		int amount = std::atoi(argv[2].c_str());
		if (amount == 0)
		{
			throw std::runtime_error("Invalid request");
		}
		else
		{
			int newAmount = auc.deposit(login, itemName, amount);
			out << "Current amount " << itemName << " " << newAmount << "\r\n";
		}
	}

	void WithdrawCommand::execute(Auction& auc, const std::string& login, const std::vector<std::string>& argv, std::ostream& out)
	{
		if (argv.size() != 3)
		{
			throw std::runtime_error("Invalid request");
		}
		std::string itemName = argv[1];
		int amount = std::atoi(argv[2].c_str());
		if (amount == 0)
		{
			throw std::runtime_error("Invalid request");
		}
		else
		{
			int newAmount = auc.withdraw(login, itemName, amount);
			out << "Current amount " << itemName << " " << newAmount << "\r\n";
		}
	}

	void SellCommand::execute(Auction& auc, const std::string& login, const std::vector<std::string>& argv, std::ostream& out)
	{
		if (argv.size() != 4)
		{
			throw std::runtime_error("Invalid request");
		}
		std::string itemName = argv[1];
		int amount = std::atoi(argv[2].c_str());
		int minPrice = std::atoi(argv[3].c_str());
		if (amount == 0 || minPrice == 0)
		{
			throw std::runtime_error("Invalid request");
		}
		else
		{
			auc.sell(login, itemName, amount, minPrice);
			out << "Sell order created " << itemName << " " << amount << " " << minPrice << "\r\n";
		}
	}

	void BuyCommand::execute(Auction& auc, const std::string& login, const std::vector<std::string>& argv, std::ostream& out)
	{
		if (argv.size() != 3)
		{
			throw std::runtime_error("Invalid request");
		}
		int orderId = std::atoi(argv[1].c_str());
		int price = std::atoi(argv[2].c_str());
		if (orderId == 0 || price == 0)
		{
			throw std::runtime_error("Invalid request");
		}
		else
		{
			auc.buy(login, orderId, price);
			out << "Buy order created " << orderId << " " << price << "\r\n";
		}
	}

	void OrderBookCommand::execute(Auction& auc, const std::string& login, const std::vector<std::string>& argv, std::ostream& out)
	{
		if (argv.size() != 1)
		{
			throw std::runtime_error("Invalid request");
		}
		auto orderBook = auc.orderBook();
		out << "OrderBook: id, itemName, amount, price" << "\r\n";
		for (const auto& x : orderBook)
		{
			out << x.first << " " << x.second.itemName << " " << x.second.amount
				<< " " << x.second.price << "\r\n";
		}
	}

	void InventoryCommand::execute(Auction& auc, const std::string& login, const std::vector<std::string>& argv, std::ostream& out)
	{
		if (argv.size() != 1)
		{
			throw std::runtime_error("Invalid request");
		}
		auto inventory = auc.inventory(login);
		out << "Inventory " << login << ": itemName, amount" << "\r\n";
		for (const auto& x : inventory)
		{
			out << x.first << " " << x.second << "\r\n";
		}
	}
}