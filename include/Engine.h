#pragma once

#include "Graphics/Renderer.h"
class Engine
{
public:
	void init();
	void run();
	void deinit();
private:
	Renderer rend;
};

