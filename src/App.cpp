#include "App.h"
#include "SDL.h"
#include "SDL_keyboard.h"
#include <map>
#include <iostream>

void Djinn::App::Init()
{
	engine.Init();
	SDL_Init(SDL_INIT_EVENTS);
}

void Djinn::App::doInput()
{
	std::map<int, bool> keyboard;
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		spdlog::error(event.type);
		std::cout << event.type;
		switch (event.type)
		{
		case SDL_KEYDOWN:
			keyboard[event.key.keysym.sym] = false;
			spdlog::error("KEY DOWN");
			break;

		case SDL_KEYUP:
			keyboard[event.key.keysym.sym] = true;
			spdlog::error("KEY UP");
			break;
		}

	}
}
//SDL_PumpEvents();
//keyboard = SDL_GetKeyState(NULL);
//if (keyboard[SDLK_Return]) spdlog::error("Return has been pressed.")


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
