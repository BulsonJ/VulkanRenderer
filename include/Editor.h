#pragma once

#include <imgui.h>

namespace Editor
{
	extern ImTextureID ViewportTexture;

	void DrawEditor();

	void DrawViewport();
	void DrawSceneGraph();
	void DrawLog();
};

