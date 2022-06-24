#pragma once

#include <vulkan/vulkan.h>

class Renderer 
{
public:
	void init();
	void deinit();

private:
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkPhysicalDevice chosenGPU;
	VkPhysicalDeviceProperties gpuProperties;
	VkDevice device;
	VkSurfaceKHR surface;

	VkQueue graphicsQueue;
	uint32_t graphicsQueueFamily;
};