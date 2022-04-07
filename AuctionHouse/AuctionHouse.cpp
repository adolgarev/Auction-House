#include <SDKDDKVer.h>
#include <iostream>
#include <boost/asio.hpp>
#include "Order.h"
#include "AsyncTimer.h"
#include "FileBackedDb.h"
#include "Auction.h"
#include "AsyncTcpServer.h"
#include "easylogging++.h"

INITIALIZE_EASYLOGGINGPP

void initLogger(int argc, char* argv[])
{
	START_EASYLOGGINGPP(argc, argv);
	el::Loggers::addFlag(el::LoggingFlag::AutoSpacing);
	el::Configurations defaultConf;
	defaultConf.setToDefault();
	defaultConf.setGlobally(el::ConfigurationType::Format, "%datetime %level %msg");
	el::Loggers::reconfigureLogger("default", defaultConf);
}

int main(int argc, char* argv[])
{
	initLogger(argc, argv);

	boost::asio::io_context io_context;

	auction::AsyncTimer<auction::Order> timer{ io_context };
	auction::FileBackedDb db{ "inventory" };
	auction::Auction auc{ db, 5, timer, 5 * 60 };

	try
	{
		int port = 2222;
		if (argc == 2)
		{
			port = std::atoi(argv[1]);
		}
		LOG(DEBUG) << "Listening at" << port;

		auction::AsyncTcpServer s(io_context, port, auc);

		io_context.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
