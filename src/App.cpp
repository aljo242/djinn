#include "App.h"
#include <map>
#include <iostream>

void Djinn::App::Init()
{
	engine.Init();
}

void Djinn::App::doInput()
{
	const auto keyState = engine.GetKeyboardState();
	const auto mouseState = engine.GetMouseState();

	if (keyState->e == DJINN_KEY_DOWN)
	{
		spdlog::error("E");
	}
}

void Djinn::App::Run()
{
	while (engine.WindowOpen())
	{
		doInput();

		engine.QueryWindowEvents();
		engine.drawFrame();
	}
}

void Djinn::App::CleanUp()
{
	engine.CleanUp();
}
