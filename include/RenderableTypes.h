#pragma once

#include <optional>
#include "glm.hpp"

namespace RenderableTypes
{
	typedef uint32_t MeshHandle;
	typedef uint32_t TextureHandle;

	struct RenderObject
	{
		MeshHandle meshHandle;
		std::optional<TextureHandle> textureHandle = {};
		std::optional<TextureHandle> normalHandle = {};

		glm::vec3 translation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
	};

	struct Texture
	{
		~Texture();

		void* ptr = nullptr;
		int texWidth;
		int texHeight;
		int texChannels;
	};

	namespace TextureUtil
	{
		void LoadTextureFromFile(const char* file, Texture& outImage);
	}
}

