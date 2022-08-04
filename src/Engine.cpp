#include "Engine.h"
#include <SDL.h>
#include <public/tracy/Tracy.hpp>
#include <backends/imgui_impl_sdl.h>
#include "Log.h"

void Engine::init() {
	ZoneScoped;

	Log::Init();
	rend.init();

	for (int i = 0; i < 4; ++i)
	{
		const RenderObject triangleObj{
			.meshName = "triangleMesh",
			.textureHandle = 0,
			.translation = { 0.25f * i,0.0f,0.25f * i },
		};
		renderObjects.push_back(triangleObj);
	}

	const RenderObject monkeyObject{
		.meshName = "fileMesh",
		.textureHandle = -1,
		.translation = { 0.0f,-0.5f,0.0f},
	};
	renderObjects.push_back(monkeyObject);
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
		rend.draw(renderObjects);
	}
}

void Engine::deinit()
{
	ZoneScoped;
	rend.deinit();
}