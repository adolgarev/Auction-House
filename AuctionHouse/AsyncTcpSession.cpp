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
							auto it = nameToCommand.find(command);
							if (command == "login" && argc == 1 && login == "")
							{
								login = tokenizer.tokens()[1];
								out << "Logged in as " << login << "\r\n";
								auc.addClientListener(login, this);
							}
							else if (command == "help" && argc == 0)
							{
								helpMessage();
							}
							else if (login != "" && it != nameToCommand.end())
							{
								it->second->execute(auc, login, tokenizer.tokens(), out);
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
	notifications.push_back(std::move(out.str()));
	if (notifications.size() == 1)
	{
		// If this is the first notification there is no async callback to send them
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
