#pragma once
#include <SDKDDKVer.h>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include "Timer.h"

namespace auction
{

	template <typename T>
	class AsyncTimer : public Timer<T>
	{
		boost::asio::io_context& io;
	public:
		AsyncTimer<T>(boost::asio::io_context& io) : io(io) {}
		virtual void setTimeout(int delaySeconds, EventListener<T>* listener, T* eventArg) override;
	};

	template<typename T>
	inline void AsyncTimer<T>::setTimeout(int delaySeconds, EventListener<T>* listener, T* eventArg)
	{
		auto t = std::make_shared<boost::asio::steady_timer>(io, boost::asio::chrono::seconds(delaySeconds));
		t->async_wait(
			[t, listener, eventArg](const boost::system::error_code&) {
				listener->on(eventArg);
			}
		);
	}
}
