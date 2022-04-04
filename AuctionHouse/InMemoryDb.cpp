#include "InMemoryDb.h"

namespace auction {

	void InMemoryDb::update(std::string login, std::string itemName, int amount)
	{
		auto key = login + '\0' + itemName;
		data[key] = amount;
	}

	int InMemoryDb::select(std::string login, std::string itemName)
	{
		auto key = login + '\0' + itemName;
		auto it = data.find(key);
		if (it == data.end())
			return 0;
		return it->second;
	}

	std::map<std::string, int> InMemoryDb::select(std::string login)
	{
		auto key = login + '\0';

		std::map<std::string, int> res{};
		for (auto it = data.lower_bound(key); it != data.end() && it->first.find(key) == 0; ++it) {
			res[it->first.substr(key.length())] = it->second;
		}
		return res;
	}
}