#include<fstream>
#include <SDKDDKVer.h>
#include <boost/asio.hpp>
#include "FileBackedMap.h"

namespace auction
{
	constexpr unsigned int maxTokenLength = 64;

	void FileBackedMap::open(const std::string& fname)
	{
		readFile(fname);
		// Create file if doesn't exist
		wf.open(fname, std::ios_base::out | std::ios_base::app);
		wf.close();
		wf.clear();
		wf.open(fname, std::ios_base::out | std::ios_base::in | std::ios_base::binary);
		if (!wf)
		{
			LOG(ERROR) << "Cannot open file" << fname;
			throw std::runtime_error("Cannot open file");
		}
	}

	void FileBackedMap::close()
	{
		wf.close();
	}

	void FileBackedMap::readFile(const std::string& fname)
	{
		data_.clear();
		std::ifstream rf(fname, std::ios_base::in | std::ios_base::binary);
		if (!rf)
		{
			// File doesn't exist, nothing to read
			return;
		}
		char key[maxTokenLength * 2 + 2];
		while (rf.peek() != EOF)
		{
			uint32_t keysize;
			rf.read((char*)&keysize, sizeof(keysize));
			keysize = boost::asio::detail::socket_ops::network_to_host_long(keysize);
			rf.read(key, keysize);
			FileBackedMapValue v;
			v.pos = rf.tellg();
			rf.read((char*)&v.value, sizeof(v.value));
			v.value = boost::asio::detail::socket_ops::network_to_host_long(v.value);
			std::string keystring{ key, keysize };
			data_[keystring] = v;
		}
		rf.close();
	}

	void FileBackedMap::set(const std::string& key, const int32_t& value)
	{
		auto it = data_.find(key);
		if (it != data_.end())
		{
			
			it->second.value = value;
			wf.seekp(it->second.pos);
			int32_t v = boost::asio::detail::socket_ops::host_to_network_long(value);
			size_t ss = sizeof(v);
			wf.write((char*)&v, sizeof(v));
		}
		else
		{
			wf.seekp(0, std::ios_base::end);
			uint32_t keysize = (uint32_t)key.length();
			keysize = boost::asio::detail::socket_ops::host_to_network_long(keysize);
			wf.write((char*)&keysize, sizeof(keysize));
			wf.write(key.c_str(), key.length());
			FileBackedMapValue val;
			val.value = value;
			val.pos = wf.tellp();
			int32_t v = boost::asio::detail::socket_ops::host_to_network_long(value);
			wf.write((char*)&v, sizeof(v));
			data_[key] = val;
		}
	}

	int32_t FileBackedMap::get(const std::string& key)
	{
		auto it = data_.find(key);
		int res{ 0 };
		if (it != data_.end())
			res = it->second.value;
		return res;
	}

	void FileBackedMap::flush()
	{
		wf.flush();
	}

	const std::map<std::string, FileBackedMapValue>& FileBackedMap::data()
	{
		return data_;
	}

}
