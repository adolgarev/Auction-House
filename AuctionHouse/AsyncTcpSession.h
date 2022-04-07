#pragma once
#include <SDKDDKVer.h>
#include <boost/asio.hpp>
#include <string>
#include <sstream>
#include <deque>
#include "Auction.h"
#include "ChunkTokenizer.h"
#include "EventListener.h"

using boost::asio::ip::tcp;

namespace auction
{
	constexpr unsigned int bufferMaxLength = 1024;
	constexpr unsigned int maxTokens = 5;
	constexpr unsigned int maxTokenLength = 64;

	class AsyncTcpSession : public std::enable_shared_from_this<AsyncTcpSession>, public EventListener<Order>
	{
	public:
		AsyncTcpSession(tcp::socket socket, Auction& auc)
			: socket_(std::move(socket)), data_(), auc(auc), tokenizer(maxTokens, maxTokenLength),
			login("")
		{
		}

		void start()
		{
			helpMessage();
			doWrite();
		}

		virtual void on(const Order*) override;

	private:
		void doRead();
		void doWrite();
		void helpMessage();
		void doWriteNotification();

		tcp::socket socket_;
		char data_[bufferMaxLength];
		Auction& auc;
		ChunkTokenizer tokenizer;
		std::string login;
		std::stringstream out;
		std::deque<std::string> notifications;
	};
}
