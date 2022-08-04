#pragma once

#include "Graphics/Renderer.h"
#include "EngineTypes.h"

class Engine
{
public:
	void init();
	void run();
	void deinit();
private:
	Renderer rend;
	std::vector<RenderObject> renderObjects;
};

