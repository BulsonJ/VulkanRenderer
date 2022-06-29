#pragma once

#include <vulkan/vulkan.h>

#include <vector>

struct SDL_Window;

namespace RenderTypes {
	struct WindowContext
	{
		SDL_Window* window = { nullptr };
		VkExtent2D extent = { 1280 , 720 };
	};

	struct CommandContext
	{
		VkCommandPool pool;
		VkCommandBuffer buffer;
	};

	template<uint32_t FRAMES>
	struct QueueContext
	{
		VkQueue queue;
		uint32_t queueFamily;
		CommandContext commands[FRAMES];
	};

	struct Swapchain
	{
		VkSwapchainKHR swapchain;
		VkFormat imageFormat;
		std::vector<VkImage> images;
		std::vector<VkImageView> imageViews;
		std::vector<VkFramebuffer> framebuffers;
	};
}
