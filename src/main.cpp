#include "hello.h"

int main()
{
#if defined(_DEBUG)
	spdlog::set_level(spdlog::level::debug);
#endif

	HelloTriangleApp app;

	try
	{
		app.run();
	}
	catch (const std::exception& e)
	{
		spdlog::error("{}", e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}