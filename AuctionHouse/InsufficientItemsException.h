#pragma once
#include <exception>
#include <string>

namespace auction
{

	struct InsufficientItemsException :
		public std::exception
	{
		std::string itemName;
		int availableAmount;
		int expectedAmount;

		InsufficientItemsException(std::string itemName, int availableAmount, int expectedAmount) : itemName(std::move(itemName)),
			availableAmount(availableAmount), expectedAmount(expectedAmount), std::exception("Insufficient items")
		{
		}
	};

}