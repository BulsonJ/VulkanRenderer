#include "PipelineBuilder.h"
#include <assert.h>
VkPipeline PipelineBuild::BuildPipeline(VkDevice device, BuildInfo buildInfo)
{
	assert(buildInfo.shaderStages.empty() == false);
	assert(buildInfo.pipelineLayout != VK_NULL_HANDLE);

	const VkPipelineViewportStateCreateInfo viewportState = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.pNext = nullptr,
		.viewportCount = 1,
		.pViewports = nullptr,
		.scissorCount = 1,
		.pScissors = nullptr,
	};

	const std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	const VkPipelineDynamicStateCreateInfo dynamicStateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.flags = 0,
		.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size()),
		.pDynamicStates = dynamicStateEnables.data(),
	};

	const VkPipelineColorBlendStateCreateInfo colorBlending = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &buildInfo.colorBlendAttachment,
	};

	const VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.pNext = nullptr,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE,
	};

	const VkGraphicsPipelineCreateInfo pipelineInfo = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = static_cast<uint32_t>(buildInfo.shaderStages.size()),
		.pStages = buildInfo.shaderStages.data(),
		.pVertexInputState = &buildInfo.vertexInputInfo,
		.pInputAssemblyState = &inputAssemblyCreateInfo,
		.pViewportState = &viewportState,
		.pRasterizationState = &buildInfo.rasterizer,
		.pDepthStencilState = &buildInfo.depthStencil,
		.pColorBlendState = &colorBlending,
		.pDynamicState = &dynamicStateInfo,
		.layout = buildInfo.pipelineLayout,
		.basePipelineHandle = VK_NULL_HANDLE,
	};

	VkPipeline newPipeline;
	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline) != VK_SUCCESS)
	{
		assert(newPipeline != VK_NULL_HANDLE);
		return VK_NULL_HANDLE; // failed to create graphics pipeline
	}
	return newPipeline;
}
