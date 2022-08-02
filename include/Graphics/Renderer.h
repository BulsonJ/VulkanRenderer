#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <glm.hpp>

#include <functional>
#include <imgui.h>
#include <unordered_map>

#include "RenderTypes.h"
#include "PipelineBuilder.h"
#include "ResourceManager.h"
#include "Mesh.h"
#include "DeletionQueue.h"

constexpr unsigned int FRAME_OVERLAP = 2U;
constexpr unsigned int MAX_OBJECTS = 100;
constexpr glm::vec3 UP_DIR = { 0.0f,1.0f,0.0f };

struct CPUImage;

struct GPUPushConstants
{
	int drawDataIndex;
};

struct GPUDrawData
{
	int transformIndex;
	int materialIndex;
	int padding[2];
};

struct GPUMaterialData
{
	int diffuseIndex = {-1};
	glm::vec4 ambient = { 1.0f, 1.0f, 1.0f, 1.0f };
	glm::vec4 diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
	glm::vec3 specular = { 1.0f, 1.0f, 1.0f };
	float shininess = { 32.0f };
};

struct GPUTransform
{
	glm::mat4 modelMatrix{1.0f};
};

struct GPUCameraData
{
	glm::mat4 view{};
	glm::mat4 proj{};
};

struct RenderObject
{
	Mesh* mesh;

	glm::vec3 translation = { 0.0f, 0.0f, 0.0f };
	glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
	glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

};

struct RenderFrame
{
	Handle<Image> renderImage;

	VkSemaphore presentSem;
	VkSemaphore	renderSem;
	VkFence renderFen;

	DeletionQueue frameDeletionQueue;

	VkDescriptorSet globalSet;
	Handle<Buffer> transformBuffer;
	Handle<Buffer> materialBuffer;
	Handle<Buffer> drawDataBuffer;

	VkDescriptorSet sceneSet;
	Handle<Buffer> cameraBuffer;
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
	void initImguiRenderpass();
	void createSwapchain();
	void recreateSwapchain();
	void destroySwapchain();

	void initGraphicsCommands();
	void initComputeCommands();
	void initSyncStructures();


	void initImgui();
	void initImguiRenderImages();
	void initShaders();
	void loadMeshes();
	void loadImages();

	void initShaderData();

	void drawObjects(VkCommandBuffer cmd);

	void uploadMesh(Mesh& mesh);
	Handle<Image> uploadImage(CPUImage& image);

	void immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);

	[[nodiscard]] int getCurrentFrameNumber() { return frameNumber % FRAME_OVERLAP; }
	[[nodiscard]] RenderFrame& getCurrentFrame() { return frame[getCurrentFrameNumber()]; }

	VkInstance instance;
	VkPhysicalDevice chosenGPU;
	VkPhysicalDeviceProperties gpuProperties;
	VkDevice device;
	DeletionQueue instanceDeletionQueue;

	VkSurfaceKHR surface;
	VmaAllocator allocator;
	VkDebugUtilsMessengerEXT debugMessenger;

	RenderTypes::QueueContext<FRAME_OVERLAP> graphics;
	RenderTypes::QueueContext<1> compute;
	RenderTypes::UploadContext uploadContext;

	RenderTypes::Swapchain swapchain;
	uint32_t currentSwapchainImage;

	VkRenderPass imguiPass;
	ImTextureID imguiRenderTexture[FRAME_OVERLAP];
	ImTextureID imguiDepthTexture;

	RenderFrame frame[FRAME_OVERLAP];
	Handle<Image> depthImage;
	int frameNumber{};

	VkDescriptorSetLayout globalSetLayout;
	VkDescriptorPool globalPool;

	VkDescriptorSetLayout sceneSetLayout;
	VkDescriptorPool scenePool;

	GPUCameraData camera;

	VkPipelineLayout defaultPipelineLayout;
	VkPipeline defaultPipeline;

	std::unordered_map<std::string, Mesh> meshes;
	//std::unordered_map<std::string, Handle<Image>> images;

	std::vector<RenderObject> renderObjects;

	std::array<Handle<Image>, 32> bindlessImages;
};