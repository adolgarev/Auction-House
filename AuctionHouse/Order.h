#pragma once

#include <string>

namespace auction {

	struct Order
	{
		uint32_t id;
		std::string loginFrom;
		std::string itemName;
		int amount;
		int price;
		std::string loginTo;
	};
}