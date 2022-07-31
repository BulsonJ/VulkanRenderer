#include "Image.h"

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <public/tracy/Tracy.hpp>
#include "Log.h"

CPUImage::~CPUImage()
{
	stbi_image_free(this->ptr);
}

void VulkanUtil::LoadImageFromFile(const char* file, CPUImage& outImage)
{
	ZoneScoped;

	stbi_uc* pixels = stbi_load(file, &outImage.texWidth, &outImage.texHeight, &outImage.texChannels, STBI_rgb_alpha);

	if (!pixels)
	{
		LOG_CORE_WARN("Failed to load texture file: " + *file);
		return;
	}

	outImage.ptr = pixels;

	return;
}
