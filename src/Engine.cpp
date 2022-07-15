#include "Engine.h"
#include <SDL.h>
#include <Tracy.hpp>

void Engine::init() {
	ZoneScoped;
	rend.init();
}

void Engine::run()
{
	bool bQuit = { false };
	SDL_Event e;

	while (!bQuit)
	{
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
			{
				bQuit = true;
			}
		}
		rend.draw();
	}
}

void Engine::deinit()
{
	ZoneScoped;
	rend.deinit();
}