#pragma once
#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>
#include "Auction.h"

namespace auction
{

	class Command
	{
		std::string name_;

	public:
		Command(const std::string& name) : name_(name) {}

		const std::string& name()
		{
			return name_;
		}

		virtual void execute(Auction& auc, const std::string& login, const std::vector<std::string>& argv, std::ostream &out) = 0;
	};

	class DepositCommand : public Command
	{
	public:
		DepositCommand() : Command("deposit") {}

		virtual void execute(Auction& auc, const std::string& login, const std::vector<std::string>& argv, std::ostream& out) override;
	};

	class WithdrawCommand : public Command
	{
	public:
		WithdrawCommand() : Command("withdraw") {}

		virtual void execute(Auction& auc, const std::string& login, const std::vector<std::string>& argv, std::ostream& out) override;
	};

	class SellCommand : public Command
	{
	public:
		SellCommand() : Command("sell") {}

		virtual void execute(Auction& auc, const std::string& login, const std::vector<std::string>& argv, std::ostream& out) override;
	};

	class BuyCommand : public Command
	{
	public:
		BuyCommand() : Command("buy") {}

		virtual void execute(Auction& auc, const std::string& login, const std::vector<std::string>& argv, std::ostream& out) override;
	};

	class OrderBookCommand : public Command
	{
	public:
		OrderBookCommand() : Command("orderBook") {}

		virtual void execute(Auction& auc, const std::string& login, const std::vector<std::string>& argv, std::ostream& out) override;
	};

	class InventoryCommand : public Command
	{
	public:
		InventoryCommand() : Command("inventory") {}

		virtual void execute(Auction& auc, const std::string& login, const std::vector<std::string>& argv, std::ostream& out) override;
	};

}

