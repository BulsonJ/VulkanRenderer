#pragma once

namespace EngineTypes
{
	struct Texture
	{
		~Texture();

		void* ptr;
		int texWidth;
		int texHeight;
		int texChannels;
	};

	namespace TextureUtil
	{
		void LoadTextureFromFile(const char* file, Texture& outImage);
	}
}

