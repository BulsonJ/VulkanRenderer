#include "Renderer.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <SDL.h>
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <VkBootstrap.h>

#include <public/tracy/Tracy.hpp>
#include <public/common/TracySystem.hpp>

#include <gtx/transform.hpp>
#include <gtx/quaternion.hpp>

#include <imgui.h>
#include <backends/imgui_impl_sdl.h>
#include <backends/imgui_impl_vulkan.h>

#include <iostream>

#include "VulkanInit.h"
#include "VulkanUtil.h"

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



	initImguiRenderpass();
	initImgui();

	initShaders();

	loadMeshes();
	loadImages();

	initShaderData();

}

void Renderer::initShaderData()
{
	ZoneScoped;
	camera.proj = glm::perspective(glm::radians(90.0f), 800.0f / 600.0f, 0.1f, 100.0f);
	camera.view =
		glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));
}

void Renderer::drawObjects(VkCommandBuffer cmd)
{	
	ZoneScoped;
	const int objectCount = 1;

	// fill buffers
	// binding 0
		//slot 0 - transform
	const glm::vec3 translation = { 0.0f, 0.0f, 0.0f };
	const glm::vec3 rotation = { 0.0f, abs(frameNumber / 120.f), 0.0f };
	const glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
	const glm::mat4 modelMatrix = glm::translate(glm::mat4{ 1.0 }, translation)
		* glm::toMat4(glm::quat(rotation))
		* glm::scale(glm::mat4{ 1.0 }, scale);
	transformData[0].modelMatrix = modelMatrix;
	memcpy(ResourceManager::ptr->GetBuffer(transformBuffer.buffer).ptr, &transformData, transformBuffer.size);
	// binding 1
		//slot 0 - camera
	camera.view =
		glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));
	memcpy(ResourceManager::ptr->GetBuffer(cameraBuffer.buffer).ptr, &camera, cameraBuffer.size);

	const Mesh* lastMesh = nullptr;
	for (int i = 0; i < objectCount; ++i)
	{
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, defaultPipeline);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, defaultPipelineLayout, 0, 1, &globalSet, 0, nullptr);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, defaultPipelineLayout, 1, 1, &sceneSet, 0, nullptr);

		GPUPushConstants constants = {};
		constants.transformIndex = 0;
		vkCmdPushConstants(cmd, defaultPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUPushConstants), &constants);

		const Mesh* currentMesh = &triangleMesh;
		if (currentMesh != lastMesh)
		{
			const VkDeviceSize offset{ 0 };
			const VkBuffer vertexBuffer = ResourceManager::ptr->GetBuffer(currentMesh->vertexBuffer.buffer).buffer;
			vkCmdBindVertexBuffers(cmd, 0, 1, &vertexBuffer, &offset);
			if (triangleMesh.hasIndices())
			{
				const VkBuffer indexBuffer = ResourceManager::ptr->GetBuffer(currentMesh->indexBuffer.buffer).buffer;
				vkCmdBindIndexBuffer(cmd, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
			}
		}

		if (currentMesh->hasIndices())
		{
			vkCmdDrawIndexed(cmd, static_cast<uint32_t>(currentMesh->indices.size()), 1, 0, 0, 0);
		}
		else
		{
			vkCmdDraw(cmd, static_cast<uint32_t>(currentMesh->vertices.size()), 1, 0, 0);
		}
	}
}

void Renderer::draw() 
{
	ZoneScoped;

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplSDL2_NewFrame(window.window);
	ImGui::NewFrame();

	ImGui::ShowDemoWindow();

	ImGui::Render();

	VK_CHECK(vkWaitForFences(device, 1, &frame.renderFen, true, 1000000000));

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

	VK_CHECK(vkResetFences(device, 1, &frame.renderFen));
	VK_CHECK(vkResetCommandBuffer(graphics.commands[getCurrentFrameNumber()].buffer, 0));

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

	drawObjects(cmd);

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

	const VkClearValue clearValue{
		.color = { 0.1f, 0.1f, 0.1f, 1.0f }
	};
	VkRenderPassBeginInfo rpInfo = VulkanInit::renderpassBeginInfo(imguiPass, window.extent, swapchain.framebuffers[swapchainImageIndex]);
	const VkClearValue clearValues[] = { clearValue };
	rpInfo.clearValueCount = 1;
	rpInfo.pClearValues = &clearValues[0];

	vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
	vkCmdEndRenderPass(cmd);

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

	VkPhysicalDeviceDescriptorIndexingFeatures descIndexFeatures{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
		.pNext = &dynamicRenderingFeature,
		.shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
		.descriptorBindingPartiallyBound = VK_TRUE,
		.descriptorBindingVariableDescriptorCount = VK_TRUE,
		.runtimeDescriptorArray = VK_TRUE,
	};

	const vkb::Device vkbDevice = deviceBuilder
		.add_pNext(&descIndexFeatures)
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

void Renderer::initImguiRenderpass() 
{
	const VkAttachmentDescription color_attachment = {
		.format = swapchain.imageFormat,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	};

	const VkAttachmentReference color_attachment_ref = {
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	const VkSubpassDescription subpass = {
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &color_attachment_ref,
	};

	VkRenderPassCreateInfo renderPassInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &color_attachment,
		.subpassCount = 1,
		.pSubpasses = &subpass,
	};

	const VkSubpassDependency dependency = {
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = 0,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
	};

	const VkSubpassDependency dependencies[2] = { dependency };

	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependencies[0];

	VK_CHECK(vkCreateRenderPass(device, &renderPassInfo, nullptr, &imguiPass));

	//create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
	VkFramebufferCreateInfo fb_info = {
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.pNext = nullptr,
		.renderPass = imguiPass,
		.attachmentCount = 1,
		.width = window.extent.width,
		.height = window.extent.height,
		.layers = 1,
	};

	const int swapchain_imagecount = static_cast<int>(swapchain.images.size());
	swapchain.framebuffers = std::vector<VkFramebuffer>(swapchain_imagecount);

	//create framebuffers for each of the swapchain image views
	for (int i = 0; i < swapchain_imagecount; i++)
	{

		VkImageView attachment = swapchain.imageViews[i];

		fb_info.pAttachments = &attachment;
		fb_info.attachmentCount = 1;

		VK_CHECK(vkCreateFramebuffer(device, &fb_info, nullptr, &swapchain.framebuffers[i]));
	}
}

void Renderer::initImgui()
{
	// TODO : Fix when IMGUI adds dynamic rendering support
	// the size of the pool is very oversize, but it's copied from imgui demo itself.
	const VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};
	
	const VkDescriptorPoolCreateInfo pool_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
		.maxSets = 1000,
		.poolSizeCount = static_cast<uint32_t>(std::size(pool_sizes)),
		.pPoolSizes = pool_sizes,
	};
	
	VkDescriptorPool imguiPool;
	VK_CHECK(vkCreateDescriptorPool(device, &pool_info, nullptr, &imguiPool));
	
	ImGui::CreateContext();
	
	ImGui_ImplSDL2_InitForVulkan(window.window);
	
	//this initializes imgui for Vulkan
	ImGui_ImplVulkan_InitInfo init_info = {
		.Instance = instance,
		.PhysicalDevice = chosenGPU,
		.Device = device,
		.Queue = graphics.queue,
		.DescriptorPool = imguiPool,
		.MinImageCount = 3,
		.ImageCount = 3,
		.MSAASamples = VK_SAMPLE_COUNT_1_BIT,
	};
	
	ImGui_ImplVulkan_Init(&init_info, imguiPass);
	
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF("../../assets/fonts/Roboto-Medium.ttf", 13);
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	//execute a gpu command to upload imgui font textures
	immediateSubmit([&](VkCommandBuffer cmd) {
		ImGui_ImplVulkan_CreateFontsTexture(cmd);
		});
	//
	////clear font textures from cpu data
	ImGui_ImplVulkan_DestroyFontUploadObjects();
	
	instanceDeletionQueue.push_function([=]{
		vkDestroyDescriptorPool(device, imguiPool, nullptr);
		ImGui_ImplVulkan_Shutdown();
		});
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

	transformBuffer = ResourceManager::ptr->CreateBuffer({ .size = sizeof(GPUTransform) * MAX_OBJECTS, .usage = BufferCreateInfo::Usage::STORAGE });
	cameraBuffer = ResourceManager::ptr->CreateBuffer({ .size = sizeof(GPUCameraData), .usage = BufferCreateInfo::Usage::UNIFORM });

	// new

	Desc::SetBindWriteInfo setWriteInfo{
		.writes = {
			Desc::BindWriteInfo{.slot = 0,.usage = Desc::Usage::STORAGE, .buffer = transformBuffer}
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
		std::cout << fileLoc << std::endl;
		assert(shader.has_value());
		std::cout << "Triangle fragment shader successfully loaded" << std::endl;
		return shader.value();
	};

	VkShaderModule vertexShader = shaderLoadFunc((std::string)"../../assets/shaders/default.vert.spv");
	VkShaderModule fragShader = shaderLoadFunc((std::string)"../../assets/shaders/default.frag.spv");

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
	ZoneScoped;
	uploadMesh(triangleMesh);
}

void Renderer::loadImages()
{
	ZoneScoped;
	CPUImage test;
	VulkanUtil::LoadImageFromFile("../../assets/textures/checkerboard.png", test);
	defaultTexture = uploadImage(test);
	return;
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
		vkDestroyFramebuffer(device, swapchain.framebuffers[i], nullptr);
		vkDestroyImageView(device, swapchain.imageViews[i], nullptr);
	}
	vkDestroyRenderPass(device, imguiPass, nullptr);

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
	ZoneScoped;
	{
		const size_t bufferSize = mesh.vertices.size() * sizeof(Vertex);

		BufferView stagingBuffer = ResourceManager::ptr->CreateBuffer(BufferCreateInfo{
			.size = bufferSize,
			.usage = BufferCreateInfo::Usage::NONE,
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

Handle<Image> Renderer::uploadImage(CPUImage& image)
{
	const VkDeviceSize imageSize = { static_cast<VkDeviceSize>(image.texWidth * image.texHeight * 4) };
	const VkFormat image_format{ VK_FORMAT_R8G8B8A8_SRGB };

	BufferView stagingBuffer = ResourceManager::ptr->CreateBuffer(BufferCreateInfo{
			.size = imageSize,
			.usage = BufferCreateInfo::Usage::NONE,
			.transfer = BufferCreateInfo::Transfer::SRC,
		});

	//copy data to buffer

	memcpy(ResourceManager::ptr->GetBuffer(stagingBuffer.buffer).ptr, image.ptr, static_cast<size_t>(imageSize));

	const VkExtent3D imageExtent{
		.width = static_cast<uint32_t>(image.texWidth),
		.height = static_cast<uint32_t>(image.texHeight),
		.depth = 1,
	};

	const VkImageCreateInfo dimg_info = VulkanInit::imageCreateInfo(image_format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, imageExtent);

	Handle<Image> newImage = ResourceManager::ptr->CreateImage(ImageCreateInfo{ .imageInfo = dimg_info, .imageType = ImageCreateInfo::ImageType::TEXTURE_2D });

	immediateSubmit([&](VkCommandBuffer cmd) {
		const VkImageSubresourceRange range{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		};

		const VkImageMemoryBarrier imageBarrier_toTransfer = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = 0,
			.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.image = ResourceManager::ptr->GetImage(newImage).image,
			.subresourceRange = range,
		};

		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toTransfer);

		const VkBufferImageCopy copyRegion = {
			.bufferOffset = 0,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1},
			.imageExtent = imageExtent,
		};

		vkCmdCopyBufferToImage(cmd, ResourceManager::ptr->GetBuffer(stagingBuffer.buffer).buffer, ResourceManager::ptr->GetImage(newImage).image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

		VkImageMemoryBarrier imageBarrier_toReadable = imageBarrier_toTransfer;

		imageBarrier_toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageBarrier_toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageBarrier_toReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageBarrier_toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toReadable);
		});

	ResourceManager::ptr->DestroyBuffer(stagingBuffer.buffer);

	return newImage;
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
