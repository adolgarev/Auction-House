#pragma once

#include "EventListener.h"

namespace auction {

	template <typename T>
	class Timer
	{
	public:
		virtual void setTimeout(int delaySeconds, EventListener<T>* listener, T* eventArg) = 0;
	};

}