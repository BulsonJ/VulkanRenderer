#include "Renderer.h"
#include <iostream>

int main() {
	Renderer test{.test = 1};
	std::cout << "Hello world\n" << test.test;
	return 0;
}