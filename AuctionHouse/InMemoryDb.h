#pragma once

#include <map>
#include <string>
#include "Db.h"

namespace auction {

	class InMemoryDb : public Db
	{
		std::map<std::string, int> data;
	public:
		virtual void update(std::string login, std::string itemName, int amount) override;
		virtual int select(std::string login, std::string itemName) override;
		virtual std::map<std::string, int> select(std::string login) override;
	};

}