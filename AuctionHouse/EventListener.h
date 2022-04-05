#pragma once

namespace auction {

	template <typename T>
	class EventListener
	{
	public:
		virtual void on(const T*) = 0;
	};

}