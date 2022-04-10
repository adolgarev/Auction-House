#pragma once

#include <string>
#include <map>
#include <fstream>
#include "easylogging++.h"

namespace auction
{
	struct FileBackedMapValue
	{
		int32_t value;
		std::streampos pos;
	};

	class FileBackedMap
	{
		void readFile(const std::string& fname);
		std::map<std::string, FileBackedMapValue> data_;
		std::ofstream wf;
	public:
		FileBackedMap() {}
		FileBackedMap(const std::string& fname)
		{
			try
			{
				open(fname);
			}
			catch (const std::exception& e)
			{
				close();
				throw e;
			}
		}
		~FileBackedMap()
		{
			close();
		}
		void open(const std::string& fname);
		void close();
		void set(const std::string& key, const int32_t& value);
		int32_t get(const std::string& key);
		const std::map<std::string, FileBackedMapValue>& data();
		void flush();
	};

}
