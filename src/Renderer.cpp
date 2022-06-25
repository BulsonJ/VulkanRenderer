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

void Renderer::init()
{
	ZoneScopedN("Renderer init");

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

}

void Renderer::draw() 
{
	
}

void Renderer::initVulkan() {
	ZoneScopedN("Vulkan init");
	vkb::InstanceBuilder builder;

	const auto inst_ret = builder.set_app_name("Vulkan Renderer")
		.request_validation_layers(true)
		.require_api_version(1, 2, 0)
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

	const vkb::Device vkbDevice = deviceBuilder
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
	ZoneScopedN("Swapchain Creation");
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

void Renderer::deinit() 
{
	ZoneScopedN("Vulkan deinit");

	vkDestroySwapchainKHR(device, swapchain.swapchain, nullptr);
	for (int i = 0; i < swapchain.imageViews.size(); i++)
	{
		vkDestroyImageView(device, swapchain.imageViews[i], nullptr);
	}

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