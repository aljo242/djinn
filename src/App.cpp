#include "App.h"
#include <map>
#include <iostream>

void Djinn::App::Init()
{
	engine.Init();
}

void Djinn::App::doInput()
{

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
