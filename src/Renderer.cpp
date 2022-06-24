#include "Renderer.h"

#include <vulkan/vulkan.h>
#include <VkBootstrap.h>

void Renderer::init() 
{
	vkb::InstanceBuilder builder;

	auto inst_ret = builder.set_app_name("Vulkan Renderer")
		.request_validation_layers(true)
		.require_api_version(1, 2, 0)
		.enable_extension("VK_EXT_debug_utils")
		.use_default_debug_messenger()
		.build();

	vkb::Instance vkb_inst = inst_ret.value();

	instance = vkb_inst.instance;
	debugMessenger = vkb_inst.debug_messenger;

	//SDL_Vulkan_CreateSurface(window, instance, &surface);

	vkb::PhysicalDeviceSelector selector{ vkb_inst };
	vkb::PhysicalDevice physicalDevice = selector
		.set_minimum_version(1, 2)
		.set_surface(surface)
		.select()
		.value();

	vkb::DeviceBuilder deviceBuilder{ physicalDevice };

	VkPhysicalDeviceHostQueryResetFeatures resetFeatures{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES,
		.pNext = nullptr,
		.hostQueryReset = VK_TRUE,
	};

	vkb::Device vkbDevice = deviceBuilder
		.add_pNext(&resetFeatures)
		.build()
		.value();

	device = vkbDevice.device;
	chosenGPU = physicalDevice.physical_device;
	gpuProperties = vkbDevice.physical_device.properties;

	graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
	graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

	//compute.queue = vkbDevice.get_queue(vkb::QueueType::compute).value();
	//compute.queueFamily = vkbDevice.get_queue_index(vkb::QueueType::compute).value();

	//VmaAllocatorCreateInfo allocatorInfo = {
	//	.physicalDevice = chosenGPU,
	//	.device = device,
	//	.instance = instance,
	//};
	//vmaCreateAllocator(&allocatorInfo, &allocator);
}

void Renderer::deinit() 
{
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkb::destroy_debug_utils_messenger(instance, debugMessenger);
	vkDestroyDevice(device, nullptr);
	vkDestroyInstance(instance, nullptr);
}