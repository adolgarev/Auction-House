#pragma once
#include <SDKDDKVer.h>
#include <memory>
#include <boost/asio.hpp>
#include "AsyncTcpSession.h"
#include "Auction.h"

using boost::asio::ip::tcp;

namespace auction
{

	class AsyncTcpServer
	{
	public:
		AsyncTcpServer(boost::asio::io_context& io_context, short port, Auction& auc)
			: acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), auc(auc)
		{
			doAccept();
		}

	private:
		void doAccept();

		tcp::acceptor acceptor_;
		Auction& auc;
	};

}