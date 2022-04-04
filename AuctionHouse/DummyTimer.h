#pragma once

#include <deque>
#include "Timer.h"

namespace auction {

	template <typename T>
	class DummyTimer : public Timer<T>
	{
		std::deque<T*> events;
	public:
		virtual void setTimeout(int delaySeconds, EventListener<T>* listener, T* eventArg) override;
		const T* popFront();
	};

	template<typename T>
	inline void DummyTimer<T>::setTimeout(int delaySeconds, EventListener<T>* listener, T* eventArg)
	{
		events.push_back(eventArg);
	}

	template<typename T>
	inline const T* DummyTimer<T>::popFront()
	{
		return events.pop_front();
	}
}