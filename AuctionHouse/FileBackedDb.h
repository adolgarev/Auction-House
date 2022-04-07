#pragma once
#include <string>
#include "Db.h"
#include "FileBackedMap.h"

namespace auction
{

	class FileBackedDb : public Db
	{
		std::string fname;
		FileBackedMap data;
		FileBackedMap wal;
		void recover();
	public:
		FileBackedDb(std::string fname) : fname(fname), data(fname), wal(fname + ".wal")
		{
			recover();
		}
		virtual void update(const std::string& login, const std::string& itemName, const int& amount) override;
		virtual int select(const std::string& login, const std::string& itemName) override;
		virtual const std::map<std::string, int> select(const std::string& login) override;
		virtual void commitTransaction() override;
		virtual void rollbackTransaction() override;
	};

}
