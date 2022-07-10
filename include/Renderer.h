#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <glm.hpp>

#include "RenderTypes.h"
#include "PipelineBuilder.h"
#include "ResourceManager.h"
#include "Mesh.h"

constexpr unsigned int FRAME_OVERLAP = 2U;
constexpr unsigned int MAX_OBJECTS = 100;

struct GPUPushConstants
{
	int transformIndex;
};

struct GPUTransform
{
	glm::mat4 modelMatrix;
};

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

	VkDescriptorSetLayout globalSetLayout;
	VkDescriptorPool globalPool;
	VkDescriptorSet globalSet;
	BufferView globalBuffer;

	VkPipelineLayout defaultPipelineLayout;
	VkPipeline defaultPipeline;

	Mesh triangleMesh{ Mesh::generate_triangle() };
};