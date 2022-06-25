#pragma once

#include <SDL.h>
#include <vulkan/vulkan.h>

struct SDL_Window;

namespace RenderTypes {
	struct WindowContext
	{
		SDL_Window* window = { nullptr };
		VkExtent2D extent = { 1920 , 1080 };
	};

	struct QueueContext
	{
		VkQueue queue;
		uint32_t queueFamily;
	};
}
