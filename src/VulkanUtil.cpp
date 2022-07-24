#include "VulkanUtil.h"

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <public/tracy/Tracy.hpp>

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
		std::cout << "Failed to load texture file " << file << std::endl;
		return;
	}

	outImage.ptr = pixels;

	return;
}
