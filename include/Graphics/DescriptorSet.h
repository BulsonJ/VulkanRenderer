#pragma once
#include <vector>

#include "ResourceManager.h"
#include "Graphics/Common.h"

namespace Desc
{
	enum Flags
	{
		VARIABLE_DESCRIPTOR = 1,
		PARTIALLY_BOUND = 2,
	};

	struct BindLayoutInfo
	{
		int slot;
		BufferView buffer;
		GFX::Stages stage;
		GFX::Buffer::Usage usage;
		Flags flags;
	};

	struct BindSetLayoutInfo
	{
		std::vector<BindLayoutInfo> bindings;
	};

	VkDescriptorSetLayout CreateDescLayout(VkDevice device, BindSetLayoutInfo& createInfo);
	VkDescriptorSet AllocateDescSet(VkDevice device, VkDescriptorPool descPool, VkDescriptorSetLayout setLayout);
	void WriteDescriptorSet(VkDevice device, VkDescriptorSet& descSet, BindSetLayoutInfo& writeInfo);
};

