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
		virtual void update(const std::string& login, const std::string& itemName, const int& amount) = 0;
		virtual int select(const std::string& login, const std::string& itemName) = 0;
		virtual const std::map<std::string, int> select(const std::string& login) = 0;

		virtual ~Db() {};
	};
}