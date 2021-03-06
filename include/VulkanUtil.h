#pragma once

#include "ResourceManager.h"

struct CPUImage
{
	~CPUImage();

	void* ptr;
	int texWidth;
	int texHeight;
	int texChannels;
};

namespace VulkanUtil
{

	void LoadImageFromFile(const char* file, CPUImage& outImage);

}

