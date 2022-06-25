#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <RenderTypes.h>

constexpr unsigned int FRAME_OVERLAP = 2U;

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
	VkSemaphore presentSem, renderSem;
	VkFence renderFen;

	RenderTypes::Swapchain swapchain;
	uint32_t currentSwapchainImage;

	int frameNumber;

};