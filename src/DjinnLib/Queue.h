#ifndef DJINN_LIB_QUEUE_INCLUDE_H
#define DJINN_LIB_QUEUE_INCLUDE_H

#include <functional>
#include <deque>

namespace Djinn
{
	class Queue
	{
	private:
		std::deque<std::function<void()>> queue;

	public:
		void PushFunction(std::function<void()>&& function)
		{
			queue.push_back(function);
		}

		void Flush()
		{
			// reverse iterate deletion queue to execute all functions
			for (auto iter = queue.rbegin(); iter != queue.rend(); ++iter)
			{
				(*iter)(); // call functions
			}

			queue.clear();
		}
	};
}

#endif // DJINN_LIB_QUEUE_INCLUDE_H