#pragma once

#include "ResourceManager.h"

class Renderer;

namespace VulkanUtil
{

	Handle<Image> LoadImageFromFile(Renderer* rend, const char* file);

}

