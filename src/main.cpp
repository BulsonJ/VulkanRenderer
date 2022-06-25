#include <Tracy.hpp>
#include <common/TracySystem.hpp>

#include "Renderer.h"
#include <SDL.h>

int main(int argc, char* argv[])
{
	ZoneScopedN("Main Function");
	tracy::SetThreadName("MainThread");

	Renderer rend;

	rend.init();
	bool bQuit = { false };
	SDL_Event e;

	while (!bQuit)
	{
		ZoneScopedN("Draw Loop")
			while (SDL_PollEvent(&e) != 0)
			{
				if (e.type == SDL_QUIT)
				{
					bQuit = true;
				}
				rend.draw();
			}
		FrameMark;
	}
	rend.deinit();

	return 0;
}