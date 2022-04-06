#pragma once
#include <SDKDDKVer.h>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <deque>
#include "Timer.h"
#include "easylogging++.h"

namespace auction
{

	template <typename T>
	class AsyncTimer : public Timer<T>
	{
		boost::asio::io_context& io;
		std::deque<boost::asio::steady_timer> timers;
	public:
		AsyncTimer<T>(boost::asio::io_context& io) : io(io) {}
		virtual void setTimeout(int delaySeconds, EventListener<T>* listener, T* eventArg) override;

		void on(EventListener<T>* listener, T* eventArg);
	};

	template<typename T>
	inline void AsyncTimer<T>::setTimeout(int delaySeconds, EventListener<T>* listener, T* eventArg)
	{
		boost::asio::steady_timer t(io, boost::asio::chrono::seconds(delaySeconds));
		t.async_wait(boost::bind(&AsyncTimer::on, this, listener, eventArg));
		timers.push_back(std::move(t));
	}

	template<typename T>
	inline void AsyncTimer<T>::on(EventListener<T>* listener, T* eventArg)
	{
		LOG(DEBUG) << "AsyncTimer event occured";
		timers.pop_front();
		listener->on(eventArg);
	}
}
