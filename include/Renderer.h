#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "RenderTypes.h"

constexpr unsigned int FRAME_OVERLAP = 2U;

struct RenderFrame
{
	VkSemaphore presentSem;
	VkSemaphore	renderSem;
	VkFence renderFen;
};

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
	void initGraphicsCommands();
	void initComputeCommands();
	void initSyncStructures();
	void initShaders();

	[[nodiscard]] int getCurrentFrameNumber() { return frameNumber % FRAME_OVERLAP; }

	VkInstance instance;
	VkPhysicalDevice chosenGPU;
	VkPhysicalDeviceProperties gpuProperties;
	VkDevice device;

	VkSurfaceKHR surface;
	VmaAllocator allocator;
	VkDebugUtilsMessengerEXT debugMessenger;

	RenderTypes::QueueContext<FRAME_OVERLAP> graphics;
	RenderTypes::QueueContext<1> compute;



	RenderTypes::Swapchain swapchain;
	uint32_t currentSwapchainImage;

	RenderFrame frame;
	int frameNumber;

};