#include "AsyncTcpServer.h"
#include "easylogging++.h"

void auction::AsyncTcpServer::doAccept()
{
	acceptor_.async_accept(
		[this](boost::system::error_code ec, tcp::socket socket)
		{
			if (!ec)
			{
				LOG(DEBUG) << "Got new connection";
				std::make_shared<AsyncTcpSession>(std::move(socket), auc)->start();
			}

			doAccept();
		});
}