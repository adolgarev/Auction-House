#include <iostream>

#include "Order.h"
#include "DummyTimer.h"
#include "InMemoryDb.h"
#include "Auction.h"

int main()
{
	auction::DummyTimer<auction::Order> timer{};
	auction::InMemoryDb db{};
	auction::Auction auc{ db, 5, timer, 5 * 60 };
	std::cout << "Hello World!\n" << auc.deposit("testLogin", "testItemName", 10);
}
