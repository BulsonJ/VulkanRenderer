#pragma once

#include <vulkan/vulkan.h>
#include <vector>

struct SDL_Window;

namespace RenderTypes {
	struct WindowContext
	{
		SDL_Window* window = { nullptr };
		VkExtent2D extent = { 1920 , 1080 };
		bool resized {false};
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

	struct UploadContext
	{
		VkFence uploadFence;
		VkCommandPool commandPool;
		VkCommandBuffer commandBuffer;
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
