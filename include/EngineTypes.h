#pragma once

#include <string>
#include <optional>

#include "glm.hpp"

namespace EngineTypes
{
	struct RenderObject
	{
		Handle<RenderMesh> meshHandle;
		std::optional<Handle<Handle<Image>>> textureHandle = {};

		glm::vec3 translation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
	};
}

