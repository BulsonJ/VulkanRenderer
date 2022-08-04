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
#include "EngineTypes.h"

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
	// TODO: Figure out padding/alignment
	//glm::vec4 ambient = { 1.0f, 1.0f, 1.0f, 1.0f };
	//glm::vec4 diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
	//glm::vec3 specular = { 1.0f, 1.0f, 1.0f };
	//float shininess = { 32.0f };
	glm::ivec4 diffuseIndex;
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

struct MaterialType
{
	VkPipeline pipeline = { VK_NULL_HANDLE };
	VkPipelineLayout pipelineLayout = { VK_NULL_HANDLE };
};

struct MaterialInstance
{
	MaterialType* matType = nullptr;

	GPUMaterialData materialData;
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
	void draw(const std::vector<RenderObject>& renderObjects);

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
	void setupScene();

	void initShaderData();

	void drawObjects(VkCommandBuffer cmd, const std::vector<RenderObject>& renderObjects);

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

	std::unordered_map<std::string, Mesh> meshes;
	//std::unordered_map<std::string, Handle<Image>> images;
	std::unordered_map<std::string, MaterialType> materials;

	//std::array<Handle<Image>, 32> bindlessImages;
	Slotmap<Handle<Image>> bindlessImages;
};