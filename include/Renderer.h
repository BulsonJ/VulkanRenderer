#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <RenderTypes.h>

class Renderer 
{
public:
	void init();
	void deinit();
	void draw();

	RenderTypes::WindowContext window;

private:
	void initVulkan();
	void createSwapchain();
	void initCommands();

	VkInstance instance;
	VkPhysicalDevice chosenGPU;
	VkPhysicalDeviceProperties gpuProperties;
	VkDevice device;

	VkSurfaceKHR surface;
	VmaAllocator allocator;
	VkDebugUtilsMessengerEXT debugMessenger;


	RenderTypes::QueueContext<2> graphics;
	RenderTypes::QueueContext<1> compute;

	RenderTypes::Swapchain swapchain;

};