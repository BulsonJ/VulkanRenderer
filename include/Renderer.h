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
	VkInstance instance;
	VkPhysicalDevice chosenGPU;
	VkPhysicalDeviceProperties gpuProperties;
	VkDevice device;

	VkSurfaceKHR surface;
	VmaAllocator allocator;
	VkDebugUtilsMessengerEXT debugMessenger;
	RenderTypes::WindowContext window;

	RenderTypes::QueueContext graphics;
	RenderTypes::QueueContext compute;

};