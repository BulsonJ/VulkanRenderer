
#include "Renderer.h"
#include <iostream>

int main(int argc, char* argv[])
{
	Renderer test;

	test.init();

	test.run();

	test.deinit();
	return 0;
}