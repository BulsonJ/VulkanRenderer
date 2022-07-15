#include "Renderer.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <SDL.h>
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <VkBootstrap.h>

#include <Tracy.hpp>
#include <common/TracySystem.hpp>

#include <imgui.h>
#include <backends/imgui_impl_sdl.h>
#include <backends/imgui_impl_vulkan.h>

#include <iostream>

#include "VulkanInit.h"

#define VK_CHECK(x)                                                 \
	do                                                              \
	{                                                               \
		VkResult err = x;                                           \
		if (err)                                                    \
		{                                                           \
			std::cout <<"Detected Vulkan error: " << err << std::endl; \
			abort();                                                \
		}                                                           \
	} while (0)

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

	initImgui();

	initShaders();



	//initImgui();
	//
	//
	loadMeshes();
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
			.color = {0.0f, 0.0f, 0.0f, 1.0f}
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

	// fill buffer

	// binding 0
		//slot 0 - transform
	memcpy(ResourceManager::ptr->GetBuffer(globalBuffer.buffer).ptr, &transformData, globalBuffer.size);
	// binding 1
		//slot 0 - camera
	memcpy(ResourceManager::ptr->GetBuffer(cameraBuffer.buffer).ptr, &camera, cameraBuffer.size);

	const VkDeviceSize offset { 0 };
	const VkBuffer vertexBuffer = ResourceManager::ptr->GetBuffer(triangleMesh.vertexBuffer.buffer).buffer;
	vkCmdBindVertexBuffers(cmd, 0, 1, &vertexBuffer, &offset);
	if (triangleMesh.hasIndices()) {
		const VkBuffer indexBuffer = ResourceManager::ptr->GetBuffer(triangleMesh.indexBuffer.buffer).buffer;
		vkCmdBindIndexBuffer(cmd, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(cmd, static_cast<uint32_t>(triangleMesh.indices.size()), 1, 0, 0, 0);
	}
	else
	{
		vkCmdDraw(cmd, static_cast<uint32_t>(triangleMesh.vertices.size()), 1, 0, 0);
	}

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

	ResourceManager::ptr = new ResourceManager(device, allocator);
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

void Renderer::initImgui()
{
	// TODO : Fix when IMGUI adds dynamic rendering support
	//// the size of the pool is very oversize, but it's copied from imgui demo itself.
	//const VkDescriptorPoolSize pool_sizes[] =
	//{
	//	{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
	//	{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
	//	{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
	//	{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
	//	{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
	//	{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
	//	{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
	//	{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
	//	{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
	//	{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
	//	{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	//};
	//
	//const VkDescriptorPoolCreateInfo pool_info = {
	//	.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
	//	.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
	//	.maxSets = 1000,
	//	.poolSizeCount = static_cast<uint32_t>(std::size(pool_sizes)),
	//	.pPoolSizes = pool_sizes,
	//};
	//
	//VkDescriptorPool imguiPool;
	//VK_CHECK(vkCreateDescriptorPool(device, &pool_info, nullptr, &imguiPool));
	//
	//ImGui::CreateContext();
	//
	//ImGui_ImplSDL2_InitForVulkan(window.window);
	//
	////this initializes imgui for Vulkan
	//ImGui_ImplVulkan_InitInfo init_info = {
	//	.Instance = instance,
	//	.PhysicalDevice = chosenGPU,
	//	.Device = device,
	//	.Queue = graphics.queue,
	//	.DescriptorPool = imguiPool,
	//	.MinImageCount = 3,
	//	.ImageCount = 3,
	//	.MSAASamples = VK_SAMPLE_COUNT_1_BIT,
	//};
	//
	////ImGui_ImplVulkan_Init(&init_info, renderPass);
	//
	////execute a gpu command to upload imgui font textures
	////immediateSubmit([&](VkCommandBuffer cmd) {
	////	ImGui_ImplVulkan_CreateFontsTexture(cmd);
	////	});
	////
	//////clear font textures from cpu data
	////ImGui_ImplVulkan_DestroyFontUploadObjects();
	//
	////instanceDeletionQueue.push_function([=]{
	//	vkDestroyDescriptorPool(device, imguiPool, nullptr);
	//	ImGui_ImplVulkan_Shutdown();
	//	});
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


	const VkCommandPoolCreateInfo uploadCommandPoolInfo = VulkanInit::commandPoolCreateInfo(graphics.queueFamily);
	vkCreateCommandPool(device, &uploadCommandPoolInfo, nullptr, &uploadContext.commandPool);

	const VkCommandBufferAllocateInfo cmdAllocInfo = VulkanInit::commandBufferAllocateInfo(uploadContext.commandPool, 1);
	vkAllocateCommandBuffers(device, &cmdAllocInfo, &uploadContext.commandBuffer);
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

	const VkFenceCreateInfo uploadFenceCreateInfo = VulkanInit::fenceCreateInfo();
	vkCreateFence(device, &uploadFenceCreateInfo, nullptr, &uploadContext.uploadFence);

	const VkSemaphoreCreateInfo semaphoreCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0
	};

	vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frame.presentSem);
	vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frame.renderSem);
}

void Renderer::initShaders() {

	ZoneScoped;
	// ------------------------ IMPROVE

	// create descriptor pool

	VkDescriptorPoolSize poolSizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10 },
	};
	VkDescriptorPoolCreateInfo poolCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = 2,
		.poolSizeCount = static_cast<uint32_t>(std::size(poolSizes)),
		.pPoolSizes = poolSizes,
	};
	vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &globalPool);
	vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &scenePool);

	// create descriptor layout

	Desc::SetBindLayoutCreateInfo globalSetBindInfo{
		.bindings = {
			Desc::BindLayoutCreateInfo{.slot = 0, .stage = Desc::Stages::ALL, .usage = Desc::Usage::STORAGE}
		}
	};

	Desc::SetBindLayoutCreateInfo sceneSetBindInfo{
		.bindings = {
			Desc::BindLayoutCreateInfo{.slot = 0, .stage = Desc::Stages::ALL, .usage = Desc::Usage::UNIFORM}
		}
	};

	globalSetLayout = Desc::CreateDescLayout(device, globalSetBindInfo);
	sceneSetLayout = Desc::CreateDescLayout(device, sceneSetBindInfo);

	// allocate descriptor set. this seems fine?

	const VkDescriptorSetAllocateInfo allocInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = nullptr,
		.descriptorPool = globalPool,
		.descriptorSetCount = 1,
		.pSetLayouts = &globalSetLayout,
	};

	vkAllocateDescriptorSets(device, &allocInfo, &globalSet);

	const VkDescriptorSetAllocateInfo sceneAllocInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = nullptr,
		.descriptorPool = scenePool,
		.descriptorSetCount = 1,
		.pSetLayouts = &sceneSetLayout,
	};

	vkAllocateDescriptorSets(device, &sceneAllocInfo, &sceneSet);

	// create buffer & write descriptor set

	globalBuffer = ResourceManager::ptr->CreateBuffer({ .size = sizeof(GPUTransform) * MAX_OBJECTS, .usage = BufferCreateInfo::Usage::STORAGE });
	cameraBuffer = ResourceManager::ptr->CreateBuffer({ .size = sizeof(GPUCameraData), .usage = BufferCreateInfo::Usage::UNIFORM });

	// new

	Desc::SetBindWriteInfo setWriteInfo{
		.writes = {
			Desc::BindWriteInfo{.slot = 0,.usage = Desc::Usage::STORAGE, .buffer = globalBuffer}
		}
	};

	Desc::WriteDescriptorSet(device, globalSet, setWriteInfo);

	Desc::SetBindWriteInfo sceneSetWriteInfo{
		.writes = {
			Desc::BindWriteInfo{.slot = 0,.usage = Desc::Usage::UNIFORM, .buffer = cameraBuffer}
		}
	};

	Desc::WriteDescriptorSet(device, sceneSet, sceneSetWriteInfo);

	// set up push constants

	const VkPushConstantRange defaultPushConstants{
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.offset = 0,
		.size = sizeof(GPUPushConstants),
	};

	VkDescriptorSetLayout setLayouts[] = { globalSetLayout, sceneSetLayout };
	VkPipelineLayoutCreateInfo defaultPipelineLayoutInfo = VulkanInit::pipelineLayoutCreateInfo();
	defaultPipelineLayoutInfo.setLayoutCount = 2;
	defaultPipelineLayoutInfo.pSetLayouts = setLayouts;
	defaultPipelineLayoutInfo.pushConstantRangeCount = 1;
	defaultPipelineLayoutInfo.pPushConstantRanges = &defaultPushConstants;

	vkCreatePipelineLayout(device, &defaultPipelineLayoutInfo, nullptr, &defaultPipelineLayout);

	auto shaderLoadFunc = [this](const std::string& fileLoc)->VkShaderModule {
		std::optional<VkShaderModule> shader = PipelineBuild::loadShaderModule(device, fileLoc.c_str());
		assert(shader.has_value());
		std::cout << "Triangle fragment shader successfully loaded" << std::endl;
		return shader.value();
	};

	VkShaderModule vertexShader = shaderLoadFunc((std::string)"../assets/shaders/default.vert.spv");
	VkShaderModule fragShader = shaderLoadFunc((std::string)"../assets/shaders/default.frag.spv");

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = VulkanInit::vertexInputStateCreateInfo();
	VertexInputDescription vertexDescription = Vertex::getVertexDescription();
	vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexDescription.attributes.size());
	vertexInputInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexDescription.bindings.size());

	// ------------------------ IMPROVE

	PipelineBuild::BuildInfo buildInfo{
		.colorBlendAttachment = VulkanInit::colorBlendAttachmentState(),
		.depthStencil = VulkanInit::depthStencilStateCreateInfo(false, false),
		.pipelineLayout = defaultPipelineLayout,
		.rasterizer = VulkanInit::rasterizationStateCreateInfo(VK_POLYGON_MODE_FILL),
		.shaderStages = {VulkanInit::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, vertexShader),
					VulkanInit::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragShader)},
		.vertexInputInfo = vertexInputInfo,
	};

	defaultPipeline = PipelineBuild::BuildPipeline(device, buildInfo);	

	vkDestroyShaderModule(device, vertexShader, nullptr);
	vkDestroyShaderModule(device, fragShader, nullptr);
}


void Renderer::loadMeshes()
{
	uploadMesh(triangleMesh);
}

void Renderer::deinit() 
{
	ZoneScoped;

	vkWaitForFences(device, 1, &frame.renderFen, true, 1000000000);

	instanceDeletionQueue.flush();

	delete ResourceManager::ptr;

	vkDestroyPipelineLayout(device, defaultPipelineLayout, nullptr);
	vkDestroyPipeline(device, defaultPipeline, nullptr);

	vkDestroyDescriptorPool(device, scenePool, nullptr);
	vkDestroyDescriptorSetLayout(device, sceneSetLayout, nullptr);
	vkDestroyDescriptorPool(device, globalPool, nullptr);
	vkDestroyDescriptorSetLayout(device, globalSetLayout, nullptr);

	vkDestroyFence(device, uploadContext.uploadFence, nullptr);
	vkDestroyCommandPool(device, uploadContext.commandPool, nullptr);

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

void Renderer::uploadMesh(Mesh& mesh)
{
	{
		const size_t bufferSize = mesh.vertices.size() * sizeof(Vertex);

		BufferView stagingBuffer = ResourceManager::ptr->CreateBuffer(BufferCreateInfo{
			.size = bufferSize,
			.usage = BufferCreateInfo::Usage::VERTEX,
			.transfer = BufferCreateInfo::Transfer::SRC,
			});

		memcpy(ResourceManager::ptr->GetBuffer(stagingBuffer.buffer).ptr, mesh.vertices.data(), bufferSize);

		mesh.vertexBuffer = ResourceManager::ptr->CreateBuffer(BufferCreateInfo{
			.size = bufferSize,
			.usage = BufferCreateInfo::Usage::VERTEX,
			.transfer = BufferCreateInfo::Transfer::DST,
			});

		const Buffer src = ResourceManager::ptr->GetBuffer(stagingBuffer.buffer);
		const Buffer dst = ResourceManager::ptr->GetBuffer(mesh.vertexBuffer.buffer);

		immediateSubmit([=](VkCommandBuffer cmd) {
			VkBufferCopy copy;
			copy.dstOffset = 0;
			copy.srcOffset = 0;
			copy.size = bufferSize;
			vkCmdCopyBuffer(cmd, src.buffer, dst.buffer, 1, &copy);
			});

		ResourceManager::ptr->DestroyBuffer(stagingBuffer.buffer);
	}

	if (!mesh.hasIndices()) return;

	{
		const size_t bufferSize = mesh.indices.size() * sizeof(Vertex);

		BufferView stagingBuffer = ResourceManager::ptr->CreateBuffer(BufferCreateInfo{
			.size = bufferSize,
			.usage = BufferCreateInfo::Usage::INDEX,
			.transfer = BufferCreateInfo::Transfer::SRC,
			});

		memcpy(ResourceManager::ptr->GetBuffer(stagingBuffer.buffer).ptr, mesh.indices.data(), bufferSize);

		mesh.indexBuffer = ResourceManager::ptr->CreateBuffer(BufferCreateInfo{
			.size = bufferSize,
			.usage = BufferCreateInfo::Usage::INDEX,
			.transfer = BufferCreateInfo::Transfer::DST,
			});

		const Buffer src = ResourceManager::ptr->GetBuffer(stagingBuffer.buffer);
		const Buffer dst = ResourceManager::ptr->GetBuffer(mesh.indexBuffer.buffer);

		immediateSubmit([=](VkCommandBuffer cmd) {
			VkBufferCopy copy;
			copy.dstOffset = 0;
			copy.srcOffset = 0;
			copy.size = bufferSize;
			vkCmdCopyBuffer(cmd, src.buffer, dst.buffer, 1, &copy);
			});

		ResourceManager::ptr->DestroyBuffer(stagingBuffer.buffer);
	}
}


void Renderer::immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function)
{
	VkCommandBuffer cmd = uploadContext.commandBuffer;

	VkCommandBufferBeginInfo cmdBeginInfo = VulkanInit::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	vkBeginCommandBuffer(cmd, &cmdBeginInfo);

	function(cmd);

	vkEndCommandBuffer(cmd);

	VkSubmitInfo submit = VulkanInit::submitInfo(&cmd);

	vkQueueSubmit(graphics.queue, 1, &submit, uploadContext.uploadFence);

	vkWaitForFences(device, 1, &uploadContext.uploadFence, true, 9999999999);
	vkResetFences(device, 1, &uploadContext.uploadFence);

	vkResetCommandPool(device, uploadContext.commandPool, 0);
}
