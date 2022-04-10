#include "easylogging++.h"
#include <stdio.h>
#include <fstream>
#include "FileBackedDb.h"

namespace auction
{

	void FileBackedDb::update(const std::string& login, const std::string& itemName, const int& amount)
	{
		auto key = login + '\0' + itemName;
		wal.set(key, amount);
	}

	int FileBackedDb::select(const std::string& login, const std::string& itemName)
	{
		auto key = login + '\0' + itemName;

		int amount{ 0 };
		auto it = wal.data().find(key);
		if (it != wal.data().end())
			amount = it->second.value;
		else
		{
			auto it = data.data().find(key);
			if (it != data.data().end())
				amount = it->second.value;
		}
		LOG(DEBUG) << "Select" << fname << login << itemName << amount;
		return amount;
	}

	const std::map<std::string, int> FileBackedDb::select(const std::string& login)
	{
		LOG(DEBUG) << "Select" << fname << login;
		auto key = login + '\0';

		std::map<std::string, int> res{};
		for (auto it = data.data().lower_bound(key); it != data.data().end() && it->first.find(key) == 0; ++it) {
			if (it->second.value > 0)
				res[it->first.substr(key.length())] = it->second.value;
		}
		return res;
	}

	void FileBackedDb::commitTransaction()
	{
		std::string walFname = fname + ".wal";
		std::string walFnameCommitted = fname + ".wal.committed";
		wal.close();
		// Atomic operation in case on same partition
		auto status = rename(walFname.c_str(), walFnameCommitted.c_str());
		if (status)
		{
			LOG(ERROR) << "Cannot rename file" << walFname << walFnameCommitted;
			throw std::runtime_error("Cannot rename file");
		}

		for (const auto& x : wal.data())
		{
			data.set(x.first, x.second.value);
		}

		// This doesn't do fsync
		data.flush();

		status = remove(walFnameCommitted.c_str());
		if (status)
		{
			LOG(ERROR) << "Cannot remove file" << walFnameCommitted;
			throw std::runtime_error("Cannot remove file");
		}
		wal.open(walFname);
	}

	void FileBackedDb::rollbackTransaction()
	{
		if (wal.data().empty())
			return;
		std::string walFname = fname + ".wal";
		wal.close();
		auto status = remove(walFname.c_str());
		if (status)
		{
			LOG(ERROR) << "Cannot remove file" << walFname;
			throw std::runtime_error("Cannot remove file");
		}
		wal.open(walFname);
	}

	void FileBackedDb::recover()
	{
		std::string walFname = fname + ".wal";
		auto status = remove(walFname.c_str());
		if (status)
		{
			LOG(DEBUG) << "No WAL file" << walFname;
		}

		std::string walFnameCommitted = fname + ".wal.committed";
		std::ifstream walFile;
		walFile.open(walFnameCommitted);
		if (!walFile)
		{
			wal.open(walFname);
			return;
		}
		walFile.close();

		wal.open(walFnameCommitted);
		for (const auto& x : wal.data())
		{
			data.set(x.first, x.second.value);
		}
		status = remove(walFnameCommitted.c_str());
		if (status)
		{
			LOG(WARNING) << "Cannot remove file" << walFnameCommitted;
		}
		wal.close();
		wal.open(walFname);
	}

}