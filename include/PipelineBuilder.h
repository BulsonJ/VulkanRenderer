#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace PipelineBuild {
	struct BuildInfo
	{
		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		VkPipelineDepthStencilStateCreateInfo depthStencil = {};
		VkPipelineLayout pipelineLayout = {};
		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {};
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	};

	VkPipeline BuildPipeline(VkDevice device, BuildInfo pipelineBuildInfo);
};
