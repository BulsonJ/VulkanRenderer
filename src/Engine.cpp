#include "Engine.h"
#include <SDL.h>
#include <public/tracy/Tracy.hpp>
#include <backends/imgui_impl_sdl.h>

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
			ImGui_ImplSDL2_ProcessEvent(&e);
			if (e.type == SDL_QUIT)
			{
				bQuit = true;
			}
			if (e.window.event == SDL_WINDOWEVENT_RESIZED || e.window.event == SDL_WINDOWEVENT_MINIMIZED)
			{
				rend.window.resized = true;
				break;
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