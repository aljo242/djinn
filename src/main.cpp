#include "App.h"

int main()
{
#if defined(_DEBUG)
	spdlog::set_level(spdlog::level::debug);
#endif

	Djinn::App app;

	try
	{
		app.Init();
		app.Run();
		app.CleanUp();
	}
	catch (const std::exception& e)
	{
		spdlog::error("{}", e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}