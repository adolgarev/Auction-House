#pragma once

#include <map>
#include <string>
#include "Db.h"

namespace auction {

	class InMemoryDb : public Db
	{
		std::string name;
		std::map<std::string, int> data;
	public:
		InMemoryDb(std::string name) : name(std::move(name)) {}
		virtual void update(const std::string& login, const std::string& itemName, const int& amount) override;
		virtual int select(const std::string& login, const std::string& itemName) override;
		virtual const std::map<std::string, int> select(const std::string& login) override;
	};

}