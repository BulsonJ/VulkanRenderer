#include "Renderer.h"
#include <iostream>

#include <VkBootstrap.h>

int main() {
	Renderer test{.test = 1};
	std::cout << "Hello world\n" << test.test;
	return 0;
}