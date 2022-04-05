#pragma once
#include <exception>
#include <string>

namespace auction
{

    struct InsufficientItemsEception :
        public std::exception
    {
        std::string itemName;
        int availableAmount;
        int expectedAmount;

        InsufficientItemsEception(std::string itemName, int availableAmount, int expectedAmount) : itemName(itemName),
            availableAmount(availableAmount), expectedAmount(expectedAmount), std::exception("Insufficient items")
        {
        }
    };

}