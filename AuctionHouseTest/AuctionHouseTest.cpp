#define BOOST_TEST_MODULE AuctionHouseTest
#include <boost/test/included/unit_test.hpp>
#include <string>
#include "../AuctionHouse/easylogging++.h"
#include "../AuctionHouse/InMemoryDb.h"
#include "../AuctionHouse/Auction.h"
#include "../AuctionHouse/DummyTimer.h"
#include "../AuctionHouse/InMemoryDb.h"
#include "../AuctionHouse/Order.h"
#include "../AuctionHouse/InsufficientItemsEception.h"
#include "../AuctionHouse/ChunkTokenizer.h"

INITIALIZE_EASYLOGGINGPP

struct InMemoryDbFixture {
	InMemoryDbFixture() : db("test") {}
	~InMemoryDbFixture() {}

	auction::InMemoryDb db;
};

BOOST_FIXTURE_TEST_SUITE(InMemoryDbTest, InMemoryDbFixture)

BOOST_AUTO_TEST_CASE(NoEntryShouldReturnZero)
{
	BOOST_TEST(0 == db.select("testLogin", "testItem"));
}

BOOST_AUTO_TEST_CASE(UpdateShouldSetValue)
{
	db.update("testLogin", "testItem", 10);
	BOOST_TEST(10 == db.select("testLogin", "testItem"));
}

BOOST_AUTO_TEST_CASE(SelectMany)
{
	db.update("testLogin1", "testItem1", 10);
	db.update("testLogin1", "testItem2", 100);
	db.update("testLogin2", "testItem2", 10);
	db.update("testLogin2", "testItem3", 10);
	auto res = db.select("testLogin1");
	BOOST_TEST(10 == res["testItem1"]);
	BOOST_TEST(100 == res["testItem2"]);
	BOOST_CHECK_MESSAGE(res.end() == res.find("testItem3"), "testItem3 in result");
}

BOOST_AUTO_TEST_SUITE_END()

struct AuctionFixture {
	AuctionFixture() : db("test"), auc(db, 5, timer, 5 * 60) {	}
	~AuctionFixture() {	}

	auction::DummyTimer<auction::Order> timer;
	auction::InMemoryDb db;
	auction::Auction auc;
};

BOOST_FIXTURE_TEST_SUITE(AuctionTest, AuctionFixture)

BOOST_AUTO_TEST_CASE(DepositTest)
{
	int newAmount = auc.deposit("testLogin", "testItem", 10);
	BOOST_TEST(10 == newAmount);

	auto inv = auc.inventory("testLogin");
	BOOST_TEST(10 == inv["testItem"]);
}

BOOST_AUTO_TEST_CASE(WithdrawTest)
{
	auc.deposit("testLogin", "testItem", 10);
	int newAmount = auc.withdraw("testLogin", "testItem", 2);
	BOOST_TEST(8 == newAmount);

	auto inv = auc.inventory("testLogin");
	BOOST_TEST(8 == inv["testItem"]);

	newAmount = auc.withdraw("testLogin", "testItem", 8);
	BOOST_TEST(0 == newAmount);
}

BOOST_AUTO_TEST_CASE(WithdrawInsufficientItemsTest)
{
	auc.deposit("testLogin", "testItem", 10);
	BOOST_CHECK_THROW(auc.withdraw("testLogin", "testItem", 20), auction::InsufficientItemsEception);
}

BOOST_AUTO_TEST_CASE(SellTest)
{
	auc.deposit("testLogin", "testItem", 10);
	auc.deposit("testLogin", auction::fundsItemName, 10);
	auc.sell("testLogin", "testItem", 8, 50);

	auto inv = auc.inventory("testLogin");
	BOOST_TEST(2 == inv["testItem"]);
	BOOST_TEST(5 == inv[auction::fundsItemName]);

	const auction::Order* order = timer.eventArg;
	BOOST_TEST(order);
	BOOST_TEST("testLogin" == order->loginFrom);
	BOOST_TEST("testItem" == order->itemName);
	BOOST_TEST(8 == order->amount);
	BOOST_TEST(50 == order->price);
	BOOST_TEST("" == order->loginTo);

	const auto& orderBook = auc.orderBook();
	BOOST_TEST(1 == orderBook.size());
	BOOST_TEST(order == &orderBook.find(order->id)->second);
}

BOOST_AUTO_TEST_CASE(SellInsufficientItemsTest)
{
	auc.deposit("testLogin", "testItem", 10);
	auc.deposit("testLogin", auction::fundsItemName, 10);
	BOOST_CHECK_THROW(auc.sell("testLogin", "testItem", 20, 50), auction::InsufficientItemsEception);

	auto inv = auc.inventory("testLogin");
	BOOST_TEST(10 == inv["testItem"]);
	BOOST_TEST(10 == inv[auction::fundsItemName]);

	const auction::Order* order = timer.eventArg;
	BOOST_TEST(nullptr == order);
}

BOOST_AUTO_TEST_CASE(SellInsufficientFundsTest)
{
	auc.deposit("testLogin", "testItem", 10);
	auc.deposit("testLogin", auction::fundsItemName, 1);
	BOOST_CHECK_THROW(auc.sell("testLogin", "testItem", 8, 50), auction::InsufficientItemsEception);

	auto inv = auc.inventory("testLogin");
	BOOST_TEST(10 == inv["testItem"]);
	BOOST_TEST(1 == inv[auction::fundsItemName]);

	const auction::Order* order = timer.eventArg;
	BOOST_TEST(nullptr == order);
}

BOOST_AUTO_TEST_CASE(SellNoBidsTest)
{
	auc.deposit("testLogin", "testItem", 10);
	auc.deposit("testLogin", auction::fundsItemName, 10);
	auc.sell("testLogin", "testItem", 8, 50);

	const auction::Order* order = timer.eventArg;
	auc.on(order);

	auto inv = auc.inventory("testLogin");
	BOOST_TEST(10 == inv["testItem"]);
	BOOST_TEST(5 == inv[auction::fundsItemName]);
}

BOOST_AUTO_TEST_CASE(BuyTest)
{
	auc.deposit("testLogin1", "testItem", 10);
	auc.deposit("testLogin1", auction::fundsItemName, 10);
	auc.sell("testLogin1", "testItem", 8, 50);
	const auction::Order* order = timer.eventArg;

	auc.deposit("testLogin2", "testItem", 5);
	auc.deposit("testLogin2", auction::fundsItemName, 100);
	auc.buy("testLogin2", order->id, 60);

	BOOST_TEST(order);
	BOOST_TEST("testLogin1" == order->loginFrom);
	BOOST_TEST("testItem" == order->itemName);
	BOOST_TEST(8 == order->amount);
	BOOST_TEST(60 == order->price);
	BOOST_TEST("testLogin2" == order->loginTo);

	auto inv = auc.inventory("testLogin1");
	BOOST_TEST(2 == inv["testItem"]);
	BOOST_TEST(5 == inv[auction::fundsItemName]);

	inv = auc.inventory("testLogin2");
	BOOST_TEST(5 == inv["testItem"]);
	BOOST_TEST(40 == inv[auction::fundsItemName]);
}

BOOST_AUTO_TEST_CASE(OrderCompletedTest)
{
	auc.deposit("testLogin1", "testItem", 10);
	auc.deposit("testLogin1", auction::fundsItemName, 10);
	auc.sell("testLogin1", "testItem", 8, 50);
	const auction::Order* order = timer.eventArg;

	auc.deposit("testLogin2", "testItem", 5);
	auc.deposit("testLogin2", auction::fundsItemName, 100);
	auc.buy("testLogin2", order->id, 60);

	auc.on(order);

	auto inv = auc.inventory("testLogin1");
	BOOST_TEST(2 == inv["testItem"]);
	BOOST_TEST(65 == inv[auction::fundsItemName]);

	inv = auc.inventory("testLogin2");
	BOOST_TEST(13 == inv["testItem"]);
	BOOST_TEST(40 == inv[auction::fundsItemName]);

	const auto& orderBook = auc.orderBook();
	BOOST_TEST(0 == orderBook.size());
}

BOOST_AUTO_TEST_CASE(SimultaneousOrdersCompletedTest)
{
	auc.deposit("testLogin1", "testItem", 10);
	auc.deposit("testLogin1", auction::fundsItemName, 10);
	auc.sell("testLogin1", "testItem", 5, 50);
	const auction::Order* order1 = timer.eventArg;
	auc.sell("testLogin1", "testItem", 3, 30);
	const auction::Order* order2 = timer.eventArg;

	auc.deposit("testLogin2", "testItem", 5);
	auc.deposit("testLogin2", auction::fundsItemName, 100);
	auc.buy("testLogin2", order1->id, 60);
	auc.buy("testLogin2", order2->id, 40);

	const auto& orderBookBefore = auc.orderBook();
	BOOST_TEST(2 == orderBookBefore.size());
	BOOST_TEST(order1 == &orderBookBefore.find(order1->id)->second);
	BOOST_TEST(order2 == &orderBookBefore.find(order2->id)->second);

	auc.on(order1);
	auc.on(order2);

	auto inv = auc.inventory("testLogin1");
	BOOST_TEST(2 == inv["testItem"]);
	BOOST_TEST(100 == inv[auction::fundsItemName]);

	inv = auc.inventory("testLogin2");
	BOOST_TEST(13 == inv["testItem"]);
	BOOST_TEST(0 == inv[auction::fundsItemName]);

	const auto& orderBookAfter = auc.orderBook();
	BOOST_TEST(0 == orderBookAfter.size());
}

BOOST_AUTO_TEST_SUITE_END()


struct ChunkTokenizerFixture {
	ChunkTokenizerFixture() : tokenizer(5, 10) {}
	~ChunkTokenizerFixture() {}

	auction::ChunkTokenizer tokenizer;
};

BOOST_FIXTURE_TEST_SUITE(ChunkTokenizerTest, ChunkTokenizerFixture)

BOOST_AUTO_TEST_CASE(ChunkTokenizerSimple)
{
	std::string data{ "a bb ccc 10\n" };
	auto res = tokenizer.parse(data.c_str(), data.length());
	BOOST_TEST(data.length() == res);
	BOOST_TEST(tokenizer.isCommandReady());
	BOOST_TEST(!tokenizer.isInErrorState());
	auto expectedTokens = std::vector<std::string>{ "a","bb","ccc","10" };
	BOOST_TEST(expectedTokens == tokenizer.tokens());
}

BOOST_AUTO_TEST_CASE(ChunkTokenizerTwoCommands)
{
	std::string data{ "a bb ccc 10\n1 22 333 aa\n" };

	auto res1 = tokenizer.parse(data.c_str(), data.length());
	BOOST_TEST(data.find('\n') + 1 == res1);
	BOOST_TEST(tokenizer.isCommandReady());
	BOOST_TEST(!tokenizer.isInErrorState());
	auto expectedTokens = std::vector<std::string>{ "a","bb","ccc","10" };
	BOOST_TEST(expectedTokens == tokenizer.tokens());

	auto res2 = tokenizer.parse(data.c_str() + res1, data.length() - res1);
	BOOST_TEST(data.length() - res1 == res2);
	BOOST_TEST(tokenizer.isCommandReady());
	BOOST_TEST(!tokenizer.isInErrorState());
	expectedTokens = std::vector<std::string>{ "1","22","333","aa" };
	BOOST_TEST(expectedTokens == tokenizer.tokens());
}

BOOST_AUTO_TEST_CASE(ChunkTokenizerTwoChunks)
{
	std::string data{ "a bb ccc 10\n" };
	for (int i = 1; i < data.length() - 1; i++)
	{
		auto res1 = tokenizer.parse(data.c_str(), i);
		BOOST_TEST(i == res1);
		BOOST_TEST(!tokenizer.isCommandReady());
		BOOST_TEST(!tokenizer.isInErrorState());

		auto res2 = tokenizer.parse(data.c_str() + i, data.length() - i);
		BOOST_TEST(data.length() == res1 + res2);
		BOOST_TEST(tokenizer.isCommandReady());
		BOOST_TEST(!tokenizer.isInErrorState());
		auto expectedTokens = std::vector<std::string>{ "a","bb","ccc","10" };
		BOOST_TEST(expectedTokens == tokenizer.tokens());
	}
}

BOOST_AUTO_TEST_CASE(ChunkTokenizerWhitespaces)
{
	std::string data{ " \ta bb ccc 10 \n" };
	auto res = tokenizer.parse(data.c_str(), data.length());
	BOOST_TEST(data.length() == res);
	BOOST_TEST(tokenizer.isCommandReady());
	BOOST_TEST(!tokenizer.isInErrorState());
	auto expectedTokens = std::vector<std::string>{ "a","bb","ccc","10" };
	BOOST_TEST(expectedTokens == tokenizer.tokens());
}

BOOST_AUTO_TEST_CASE(ChunkTokenizerTooManyTokens)
{
	std::string data{ "a bb ccc 10 d e\n" };
	auto res = tokenizer.parse(data.c_str(), data.length());
	BOOST_TEST(data.length() == res);
	BOOST_TEST(!tokenizer.isCommandReady());
	BOOST_TEST(tokenizer.isInErrorState());

	tokenizer.clearErrorState();
	data = "a bb ccc 10 d e f f f f\n";
	res = tokenizer.parse(data.c_str(), data.length());
	BOOST_TEST(data.length() == res);
	BOOST_TEST(!tokenizer.isCommandReady());
	BOOST_TEST(tokenizer.isInErrorState());

	tokenizer.clearErrorState();
	data = "a bb ccc 10\n";
	res = tokenizer.parse(data.c_str(), data.length());
	BOOST_TEST(data.length() == res);
	BOOST_TEST(tokenizer.isCommandReady());
	BOOST_TEST(!tokenizer.isInErrorState());
	auto expectedTokens = std::vector<std::string>{ "a","bb","ccc","10" };
	BOOST_TEST(expectedTokens == tokenizer.tokens());
}

BOOST_AUTO_TEST_CASE(ChunkTokenizerTokenTooLong)
{
	std::string data{ "a bb ccc eeeeeeeeeeeeeeeeeeee\n" };
	auto res = tokenizer.parse(data.c_str(), data.length());
	BOOST_TEST(data.length() == res);
	BOOST_TEST(!tokenizer.isCommandReady());
	BOOST_TEST(tokenizer.isInErrorState());

	tokenizer.clearErrorState();
	data = "a bb ccccccccccccccccccccc 10\n";
	res = tokenizer.parse(data.c_str(), data.length());
	BOOST_TEST(data.length() == res);
	BOOST_TEST(!tokenizer.isCommandReady());
	BOOST_TEST(tokenizer.isInErrorState());

	tokenizer.clearErrorState();
	data = "a bb ccc 10\n";
	res = tokenizer.parse(data.c_str(), data.length());
	BOOST_TEST(data.length() == res);
	BOOST_TEST(tokenizer.isCommandReady());
	BOOST_TEST(!tokenizer.isInErrorState());
	auto expectedTokens = std::vector<std::string>{ "a","bb","ccc","10" };
	BOOST_TEST(expectedTokens == tokenizer.tokens());
}

BOOST_AUTO_TEST_SUITE_END()