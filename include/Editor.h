#pragma once

#include <imgui.h>

namespace Editor
{
	extern ImTextureID ViewportTexture;
	extern ImTextureID ViewportDepthTexture;

	void DrawEditor();

	void DrawViewport();
	void DrawViewportDepth();
	void DrawSceneGraph();
	void DrawLog();
};

