#pragma once

#include <string>
#include <map>
#include "DbTransaction.h"

namespace auction {

	class Db
	{
	public:
		virtual DbTransaction startTransaction();
		virtual void commitTransaction();
		virtual void rollbackTransaction();
		virtual void update(std::string login, std::string itemName, int amount) = 0;
		virtual int select(std::string login, std::string itemName) = 0;
		virtual std::map<std::string, int> select(std::string login) = 0;

		virtual ~Db() {};
	};
}