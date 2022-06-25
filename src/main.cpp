
#include "Renderer.h"

#include <Tracy.hpp>

int main(int argc, char* argv[])
{
	ZoneScoped;
	tracy::SetThreadName("MainThread");

	Renderer rend;

	rend.init();
	rend.run();
	rend.deinit();
	return 0;
}