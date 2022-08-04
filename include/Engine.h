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
	void setupScene();

	Renderer rend;
	std::vector<EngineTypes::RenderObject> renderObjects;
};

