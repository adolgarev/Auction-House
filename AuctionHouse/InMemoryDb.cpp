#include "easylogging++.h"
#include "InMemoryDb.h"

namespace auction {

	void InMemoryDb::update(std::string login, std::string itemName, int amount)
	{
		LOG(DEBUG) << "Update" << name << login << itemName << amount;
		auto key = login + '\0' + itemName;
		data[key] = amount;
	}

	int InMemoryDb::select(std::string login, std::string itemName)
	{
		auto key = login + '\0' + itemName;

		auto it = data.find(key);
		int amount{ 0 };
		if (it != data.end())
			amount = it->second;
		LOG(DEBUG) << "Select" << name << login << itemName << amount;
		return amount;
	}

	const std::map<std::string, int> InMemoryDb::select(std::string login)
	{
		LOG(DEBUG) << "Select" << name << login;
		auto key = login + '\0';

		std::map<std::string, int> res{};
		for (auto it = data.lower_bound(key); it != data.end() && it->first.find(key) == 0; ++it) {
			res[it->first.substr(key.length())] = it->second;
		}
		return res;
	}
}