#pragma once

#include <vulkan/vulkan.h>

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

	VkQueue graphicsQueue;
	uint32_t graphicsQueueFamily;
};