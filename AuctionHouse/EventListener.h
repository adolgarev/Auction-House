#pragma once

namespace auction {

	template <typename T>
	class EventListener
	{
	public:
		virtual void on(T*) = 0;
	};

}