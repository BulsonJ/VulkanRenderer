#pragma once

#include <string>

#include "glm.hpp"

namespace EngineTypes
{
	struct RenderObject
	{
		Handle<RenderMesh> meshHandle;
		int32_t textureHandle;

		glm::vec3 translation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
	};
}

