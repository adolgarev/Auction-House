#include <iostream>

#include "Order.h"
#include "DummyTimer.h"
#include "InMemoryDb.h"
#include "Auction.h"
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

	auction::DummyTimer<auction::Order> timer{};
	auction::InMemoryDb db{ "inventory" };
	auction::Auction auc{ db, 5, timer, 5 * 60 };
	std::cout << "Hello World!\n" << auc.deposit("testLogin", "testItemName", 10);
}
