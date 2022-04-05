#pragma once

#include "Timer.h"

namespace auction {

	template <typename T>
	class DummyTimer : public Timer<T>
	{
	public:
		T* eventArg;
		DummyTimer<T>() : eventArg(nullptr) {}
		virtual void setTimeout(int delaySeconds, EventListener<T>* listener, T* eventArg) override;
	};

	template<typename T>
	inline void DummyTimer<T>::setTimeout(int delaySeconds, EventListener<T>* listener, T* eventArg)
	{
		this->eventArg = eventArg;
	}
}