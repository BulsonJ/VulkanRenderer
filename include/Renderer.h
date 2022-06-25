#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <RenderTypes.h>

class Renderer 
{
public:
	void init();
	void deinit();
	void run();

private:
	RenderTypes::WindowContext window;

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