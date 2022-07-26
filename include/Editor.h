#pragma once

#include <imgui.h>
#include "spdlog/sinks/ostream_sink.h"

namespace Editor
{
	extern ImTextureID ViewportTexture;
	extern ImTextureID ViewportDepthTexture;
	extern std::ostringstream _oss;

	void DrawEditor();

	void DrawViewportWindow();
	void DrawViewport();
	void DrawViewportDepth();
	void DrawSceneGraph();
	void DrawLog();


};

