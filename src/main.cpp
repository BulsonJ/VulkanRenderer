
#include "Renderer.h"
#include <iostream>

int main(int argc, char* argv[])
{
	Renderer rend;

	rend.init();

	rend.run();

	rend.deinit();
	return 0;
}