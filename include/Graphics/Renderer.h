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

namespace EngineTypes
{
	struct MeshDesc;
	struct RenderObject;
	struct Texture;
}

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
	glm::vec4 diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
	glm::vec3 specular = { 1.0f, 1.0f, 1.0f };
	float shininess = { 32.0f };
	glm::ivec4 diffuseIndex;
};

struct GPUTransform
{
	glm::mat4 modelMatrix{};
};

struct GPUDirectionalLightData
{
	glm::vec4 direction = { -0.2f, -1.0f, -0.3f, 1.0f };
	glm::vec4 color = { 1.0f,1.0f,1.0f,1.0f };
	glm::vec4 ambientColor = { 0.7f, 0.7f, 0.7f, 1.0f };
};

struct GPUCameraData
{
	glm::mat4 view{};
	glm::mat4 proj{};
	glm::vec4 pos{};
};

struct VertexInputDescription
{
	std::vector<VkVertexInputBindingDescription> bindings;
	std::vector<VkVertexInputAttributeDescription> attributes;

	VkPipelineVertexInputStateCreateFlags flags = 0;
};

struct RenderMesh
{
	EngineTypes::MeshDesc meshDesc;
	Handle<Buffer> vertexBuffer;
	Handle<Buffer> indexBuffer;

	static VertexInputDescription getVertexDescription();
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
	Handle<Buffer> dirLightBuffer;
};

class Renderer 
{
public:
	void init();
	void deinit();

	void draw(const std::vector<EngineTypes::RenderObject>& renderObjects);
	Handle<RenderMesh> uploadMesh(const EngineTypes::MeshDesc& mesh);
	Handle<Handle<Image>> uploadTexture(const EngineTypes::Texture& texture);

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

	void initShaderData();

	void drawObjects(VkCommandBuffer cmd, const std::vector<EngineTypes::RenderObject>& renderObjects);

	Handle<Image> uploadTextureInternal(const EngineTypes::Texture& image);

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
	GPUDirectionalLightData sunlight;

	Slotmap<RenderMesh> meshes;
	//std::unordered_map<std::string, Handle<Image>> images;
	std::unordered_map<std::string, MaterialType> materials;

	Slotmap<Handle<Image>> bindlessImages;
};