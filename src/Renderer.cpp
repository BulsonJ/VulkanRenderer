#include "Renderer.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <SDL.h>
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <VkBootstrap.h>
#include <Tracy.hpp>
#include <common/TracySystem.hpp>
#include <iostream>

#include "VulkanInit.h"

void Renderer::init()
{
	ZoneScoped;

	SDL_Init(SDL_INIT_VIDEO);

	const SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);

	window.window = SDL_CreateWindow(
		"Vulkan Renderer",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		window.extent.width,
		window.extent.height,
		window_flags
	);

	initVulkan();
	createSwapchain();
	initGraphicsCommands();
	initComputeCommands();
	initSyncStructures();

	initShaders();


	//initImgui();
	//
	//initDescriptorLayouts();
	//initDescriptors();
	//initPipelines();
	//initComputePipeline();
	//
	//loadMeshes();
	//loadImages();
	//
	//initScene();
	//buildComputeCommandBuffer();

}

void Renderer::draw() 
{
	ZoneScoped;
	vkWaitForFences(device, 1, &frame.renderFen, true, 1000000000);


	uint32_t swapchainImageIndex;
	VkResult result = vkAcquireNextImageKHR(device, swapchain.swapchain, 1000000000, frame.presentSem, nullptr, &swapchainImageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		// resize
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	vkResetFences(device, 1, &frame.renderFen);
	vkResetCommandBuffer(graphics.commands[getCurrentFrameNumber()].buffer, 0);

	const VkCommandBuffer cmd = graphics.commands[getCurrentFrameNumber()].buffer;

	const VkCommandBufferBeginInfo cmdBeginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = nullptr,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		.pInheritanceInfo = nullptr,
	};

	vkBeginCommandBuffer(cmd, &cmdBeginInfo);

	const VkViewport viewport{
		.x = 0.0f,
		.y = 0.0f,
		.width = static_cast<float>(window.extent.width),
		.height = static_cast<float>(window.extent.height),
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};

	const VkRect2D scissor{
		.offset = {.x = 0,.y = 0},
		.extent = window.extent
	};

	vkCmdSetViewport(cmd, 0, 1, &viewport);
	vkCmdSetScissor(cmd, 0, 1, &scissor);

	const VkImageMemoryBarrier presentImgMemBarrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.image = swapchain.images[swapchainImageIndex],
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		}
	};

	vkCmdPipelineBarrier(
		cmd,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		0,
		0,
		nullptr,
		0,
		nullptr,
		1,
		&presentImgMemBarrier
	);

	const VkRenderingAttachmentInfo colorAttachInfo{
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageView = swapchain.imageViews[swapchainImageIndex],
		.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.clearValue = { 
			.color = {0.1f, 0.1f, abs(sin(frameNumber / 120.f)), 1.0f}
		}
	};

	const VkRenderingInfo renderInfo{
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.renderArea = scissor,
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachInfo
	};
	vkCmdBeginRendering(cmd, &renderInfo);


	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, defaultPipeline);
	vkCmdDraw(cmd, 3, 1, 0, 0);


	vkCmdEndRendering(cmd);

	const VkImageMemoryBarrier imgMemBarrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		.image = swapchain.images[swapchainImageIndex],
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		}
	};

	vkCmdPipelineBarrier(
		cmd,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		0,
		0,
		nullptr,
		0,
		nullptr,
		1,
		&imgMemBarrier
	);

	vkEndCommandBuffer(cmd);

	const VkPipelineStageFlags waitStage { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	const VkSubmitInfo submit = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = nullptr,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &frame.presentSem,
		.pWaitDstStageMask = &waitStage,
		.commandBufferCount = 1,
		.pCommandBuffers = &cmd,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &frame.renderSem,
	};

	vkQueueSubmit(graphics.queue, 1, &submit, frame.renderFen);

	const VkPresentInfoKHR presentInfo = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = nullptr,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &frame.renderSem,
		.swapchainCount = 1,
		.pSwapchains = &swapchain.swapchain,
		.pImageIndices = &swapchainImageIndex,
	};

	vkQueuePresentKHR(graphics.queue, &presentInfo);
	FrameMark;
	frameNumber++;
}

void Renderer::initVulkan() {
	ZoneScoped;
	vkb::InstanceBuilder builder;

	const auto inst_ret = builder.set_app_name("Vulkan Renderer")
		.request_validation_layers(true)
		.require_api_version(1, 3, 0)
		.enable_extension("VK_EXT_debug_utils")
		.use_default_debug_messenger()
		.build();

	const vkb::Instance vkb_inst = inst_ret.value();

	instance = vkb_inst.instance;
	debugMessenger = vkb_inst.debug_messenger;

	SDL_Vulkan_CreateSurface(window.window, instance, &surface);

	vkb::PhysicalDeviceSelector selector{ vkb_inst };
	const vkb::PhysicalDevice physicalDevice = selector
		.set_minimum_version(1, 2)
		.set_surface(surface)
		.select()
		.value();

	vkb::DeviceBuilder deviceBuilder{ physicalDevice };

	VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeature{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
		.dynamicRendering = VK_TRUE,
	};

	const vkb::Device vkbDevice = deviceBuilder
		.add_pNext(&dynamicRenderingFeature)
		.build()
		.value();

	device = vkbDevice.device;
	chosenGPU = physicalDevice.physical_device;
	gpuProperties = vkbDevice.physical_device.properties;

	graphics.queue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
	graphics.queueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
	compute.queue = vkbDevice.get_queue(vkb::QueueType::compute).value();
	compute.queueFamily = vkbDevice.get_queue_index(vkb::QueueType::compute).value();

	const VmaAllocatorCreateInfo allocatorInfo = {
		.physicalDevice = chosenGPU,
		.device = device,
		.instance = instance,
	};
	vmaCreateAllocator(&allocatorInfo, &allocator);
}

void Renderer::createSwapchain()
{
	ZoneScoped;
	vkb::SwapchainBuilder swapchainBuilder{ chosenGPU,device,surface };

	vkb::Swapchain vkbSwapchain = swapchainBuilder
		.use_default_format_selection()
		.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
		.set_desired_extent(window.extent.width, window.extent.height)
		.build()
		.value();

	swapchain.swapchain = vkbSwapchain.swapchain;
	swapchain.images = vkbSwapchain.get_images().value();
	swapchain.imageViews = vkbSwapchain.get_image_views().value();
	swapchain.imageFormat = vkbSwapchain.image_format;
}

void Renderer::initGraphicsCommands()
{
	ZoneScoped;
	const VkCommandPoolCreateInfo graphicsCommandPoolCreateInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = graphics.queueFamily
	};

	for (int i = 0; i < 2; ++i)
	{
		VkCommandPool* commandPool = &graphics.commands[i].pool;

		vkCreateCommandPool(device, &graphicsCommandPoolCreateInfo, nullptr, commandPool);

		const VkCommandBufferAllocateInfo bufferAllocInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = nullptr,
			.commandPool = *commandPool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1
		};

		vkAllocateCommandBuffers(device, &bufferAllocInfo, &graphics.commands[i].buffer);
	}
}

void Renderer::initComputeCommands(){
	ZoneScoped;
	const VkCommandPoolCreateInfo computeCommandPoolCreateInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = compute.queueFamily
	};

	VkCommandPool* commandPool = &compute.commands[0].pool;

	vkCreateCommandPool(device, &computeCommandPoolCreateInfo, nullptr, commandPool);

	const VkCommandBufferAllocateInfo bufferAllocInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = nullptr,
		.commandPool = *commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1
	};

	vkAllocateCommandBuffers(device, &bufferAllocInfo, &compute.commands[0].buffer);

}

void Renderer::initSyncStructures()
{
	ZoneScoped;
	const VkFenceCreateInfo fenceCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT
	};

	vkCreateFence(device, &fenceCreateInfo, nullptr, &frame.renderFen);

	const VkSemaphoreCreateInfo semaphoreCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0
	};

	vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frame.presentSem);
	vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frame.renderSem);
}

void Renderer::initShaders() {


	// ------------------------ IMPROVE

	const VkDescriptorSetLayoutBinding configBind = VulkanInit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL, 0);

	VkDescriptorSetLayoutCreateInfo descSetCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = 1,
		.pBindings = &configBind,
	};
	vkCreateDescriptorSetLayout(device, &descSetCreateInfo, nullptr, &globalSetLayout);

	VkDescriptorPoolSize poolSizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
	};
	VkDescriptorPoolCreateInfo poolCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = 2,
		.poolSizeCount = static_cast<uint32_t>(std::size(poolSizes)),
		.pPoolSizes = poolSizes,
	};
	vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &globalPool);

	VkDescriptorSetLayout setLayouts[] = { globalSetLayout };
	VkPipelineLayoutCreateInfo defaultPipelineLayoutInfo = VulkanInit::pipelineLayoutCreateInfo();
	defaultPipelineLayoutInfo.setLayoutCount = 1;
	defaultPipelineLayoutInfo.pSetLayouts = setLayouts;

	vkCreatePipelineLayout(device, &defaultPipelineLayoutInfo, nullptr, &defaultPipelineLayout);

	auto shaderLoadFunc = [this](const std::string& fileLoc)->VkShaderModule {
		std::optional<VkShaderModule> shader = PipelineBuild::loadShaderModule(device, fileLoc.c_str());
		assert(shader.has_value());
		std::cout << "Triangle fragment shader successfully loaded" << std::endl;
		return shader.value();
	};

	VkShaderModule vertexShader = shaderLoadFunc((std::string)"../assets/shaders/default.vert.spv");
	VkShaderModule fragShader = shaderLoadFunc((std::string)"../assets/shaders/default.frag.spv");

	// ------------------------ IMPROVE

	PipelineBuild::BuildInfo buildInfo{
		.colorBlendAttachment = VulkanInit::colorBlendAttachmentState(),
		.depthStencil = VulkanInit::depthStencilStateCreateInfo(false, false),
		.pipelineLayout = defaultPipelineLayout,
		.rasterizer = VulkanInit::rasterizationStateCreateInfo(VK_POLYGON_MODE_FILL),
		.shaderStages = {VulkanInit::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, vertexShader),
					VulkanInit::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragShader)},
		.vertexInputInfo = VulkanInit::vertexInputStateCreateInfo(),
	};

	defaultPipeline = PipelineBuild::BuildPipeline(device, buildInfo);

	vkDestroyShaderModule(device, vertexShader, nullptr);
	vkDestroyShaderModule(device, fragShader, nullptr);
}

void Renderer::deinit() 
{
	ZoneScoped;

	vkWaitForFences(device, 1, &frame.renderFen, true, 1000000000);

	vkDestroyPipelineLayout(device, defaultPipelineLayout, nullptr);
	vkDestroyPipeline(device, defaultPipeline, nullptr);

	vkDestroyDescriptorPool(device, globalPool, nullptr);
	vkDestroyDescriptorSetLayout(device, globalSetLayout, nullptr);

	vkDestroySemaphore(device, frame.presentSem, nullptr);
	vkDestroySemaphore(device, frame.renderSem, nullptr);
	vkDestroyFence(device, frame.renderFen, nullptr);

	for (int i = 0; i < swapchain.imageViews.size(); i++)
	{
		vkDestroyImageView(device, swapchain.imageViews[i], nullptr);
	}
	vkDestroySwapchainKHR(device, swapchain.swapchain, nullptr);

	vkDestroyCommandPool(device, graphics.commands[0].pool, nullptr);
	vkDestroyCommandPool(device, graphics.commands[1].pool, nullptr);
	vkDestroyCommandPool(device, compute.commands[0].pool, nullptr);

	vmaDestroyAllocator(allocator);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkb::destroy_debug_utils_messenger(instance, debugMessenger);
	vkDestroyDevice(device, nullptr);
	vkDestroyInstance(instance, nullptr);

	SDL_DestroyWindow(window.window);
}