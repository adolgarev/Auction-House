#include "AsyncTcpSession.h"
#include "easylogging++.h"

void auction::AsyncTcpSession::doRead()
{
	auto self(shared_from_this());
	socket_.async_read_some(boost::asio::buffer(data_, bufferMaxLength),
		[this, self](boost::system::error_code ec, std::size_t length)
		{
			out.clear();
			out.str(std::string());
			if (!ec)
			{
				std::size_t res{ 0 };
				while (res < length)
				{
					res += tokenizer.parse(data_ + res, length - res);
					if (tokenizer.isInErrorState())
					{
						out << "Invalid request" << "\r\n";
						tokenizer.clearErrorState();
						helpMessage();
					}
					else if (tokenizer.isCommandReady())
					{
						if (tokenizer.tokens().size() == 0)
							continue;
						try
						{
							const std::string& command = tokenizer.tokens().front();
							const size_t argc = tokenizer.tokens().size() - 1;
							if (command == "login" && argc == 1 && login == "")
							{
								login = tokenizer.tokens()[1];
								out << "Logged in as " << login << "\r\n";
								auc.addClientListener(login, this);
							}
							else if (command == "deposit" && argc == 2 && login != "")
							{
								std::string itemName = tokenizer.tokens()[1];
								int amount = std::atoi(tokenizer.tokens()[2].c_str());
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
							else if (command == "withdraw" && argc == 2 && login != "")
							{
								std::string itemName = tokenizer.tokens()[1];
								int amount = std::atoi(tokenizer.tokens()[2].c_str());
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
							else if (command == "sell" && argc == 3 && login != "")
							{
								std::string itemName = tokenizer.tokens()[1];
								int amount = std::atoi(tokenizer.tokens()[2].c_str());
								int minPrice = std::atoi(tokenizer.tokens()[3].c_str());
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
							else if (command == "buy" && argc == 2 && login != "")
							{
								int orderId = std::atoi(tokenizer.tokens()[1].c_str());
								int price = std::atoi(tokenizer.tokens()[2].c_str());
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
							else if (command == "orderBook" && argc == 0 && login != "")
							{
								auto orderBook = auc.orderBook();
								out << "OrderBook: id, itemName, amount, price" << "\r\n";
								for (const auto& x : orderBook)
								{
									out << x.first << " " << x.second.itemName << " " << x.second.amount
										<< " " << x.second.price << "\r\n";
								}
							}
							else if (command == "inventory" && argc == 0 && login != "")
							{
								auto inventory = auc.inventory(login);
								out << "Inventory " << login << ": itemName, amount" << "\r\n";
								for (const auto& x : inventory)
								{
									out << x.first << " " << x.second << "\r\n";
								}
							}
							else if (command == "help" && argc == 0)
							{
								helpMessage();
							}
							else
							{
								throw std::runtime_error("Invalid request");
							}
						}
						catch (const std::exception& e)
						{
							out << e.what() << "\r\n";
							helpMessage();
						}
					}
				}

				if (out.str().length() > 0)
					doWrite();
				else
					doRead();
			}
			else
			{
				LOG(DEBUG) << "Disconnected";
				if (login != "")
					auc.removeClientLitener(login);
			}
		});
}

void auction::AsyncTcpSession::doWrite()
{
	auto self(shared_from_this());
	boost::asio::async_write(socket_, boost::asio::buffer(out.str().c_str(), out.str().length()),
		[this, self](boost::system::error_code ec, std::size_t /*length*/)
		{
			if (!ec)
			{
				doRead();
			}
			else
			{
				LOG(DEBUG) << "Disconnected";
				if (login != "")
					auc.removeClientLitener(login);
			}
		});
}

void auction::AsyncTcpSession::on(const Order* order)
{
	std::stringstream out;
	if (login == order->loginFrom)
	{
		if (order->loginTo != "")
		{
			out << "Sell order completed";
		}
		else
		{
			out << "Sell order expired";
		}
	}
	else
	{
		out << "Buy order completed";
	}

	out << ": order id " << order->id << ", amount " << order->amount << ", item "
		<< order->itemName << ", price " << order->price << "\r\n";
	notifications.push_back(out.str());
	if (notifications.size() == 1)
	{
		doWriteNotification();
	}
}

void auction::AsyncTcpSession::doWriteNotification()
{
	auto it = notifications.begin();
	auto self(shared_from_this());
	boost::asio::async_write(socket_, boost::asio::buffer(it->c_str(), it->length()),
		[this, self](boost::system::error_code ec, std::size_t /*length*/)
		{
			notifications.pop_front();
			if (!ec && notifications.size() > 0)
			{
				doWriteNotification();
			}
		});
}

void auction::AsyncTcpSession::helpMessage()
{
	out << "Welcome to the Auction house" << "\r\n";
	out << "Please write one command per line, arguments are separated with spaces, amount and price are integers" << "\r\n";
	out << "Please login first" << "\r\n";
	out << "To deposit or withdraw funds use item name \"funds\", e.g. \"deposit funds 10\"" << "\r\n";
	out << "The following commands are available:" << "\r\n";
	out << "  login LOGIN" << "\r\n";
	out << "  deposit ITEMNAME AMOUNT" << "\r\n";
	out << "  withdraw ITEMNAME AMOUNT" << "\r\n";
	out << "  sell ITEMNAME AMOUNT PRICE" << "\r\n";
	out << "  buy ORDERID PRICE" << "\r\n";
	out << "  orderBook" << "\r\n";
	out << "  inventory" << "\r\n";
	out << "  help" << "\r\n";
}
