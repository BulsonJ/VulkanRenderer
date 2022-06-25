#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

class Renderer 
{
public:
	void init();
	void deinit();

private:
	VkExtent2D windowExtent = { 1920 , 1080 };

	struct SDL_Window* window = nullptr;

	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkPhysicalDevice chosenGPU;
	VkPhysicalDeviceProperties gpuProperties;
	VkDevice device;
	VkSurfaceKHR surface;
	VmaAllocator allocator;

	VkQueue graphicsQueue;
	uint32_t graphicsQueueFamily;

};