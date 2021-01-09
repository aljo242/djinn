#ifndef APP_INCLUDE_H
#define APP_INCLUDE_H

#include "VulkanEngine.h"

namespace Djinn
{
	class App
	{
	public:
		void Init();
		void Run();
		void CleanUp();
		
	private:
		VulkanEngine engine;
		void doInput();

	};
}

#endif // APP_INCLUDE_H