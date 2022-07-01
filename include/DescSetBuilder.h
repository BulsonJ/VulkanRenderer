#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include "VulkanTypes.h"

namespace DescriptorSet
{
	enum class BufferType
	{
		UNIFORM,
		STORAGE,
	};

	enum class ShaderStage
	{
		VERTEX,
		PIXEL,
		COMPUTE,
	};

	struct DescSetBind
	{
		BufferType bufferType;
		ShaderStage shaderStage;
	};

	struct LayoutBuildInfo
	{
		std::vector<DescSetBind> bindings;
	};

	VkDescriptorSetLayout CreateDescriptorSetLayout(VkDevice device, LayoutBuildInfo buildInfo);

	struct SetWriteInfo
	{
		BufferType bufferType;
		VkDescriptorSet* descriptorSet;
		VkDescriptorBufferInfo bufferInfo;
	};

	struct WriteInfo {
		std::vector<SetWriteInfo> descriptorSetWrites;
	};

	void WriteDescriptorSet(VkDevice device, WriteInfo writeInfo);
};

