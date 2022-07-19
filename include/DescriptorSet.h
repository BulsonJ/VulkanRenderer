#pragma once
#include <vector>
#include "ResourceManager.h"

namespace Desc
{
	enum Stages
	{
		VERTEX = 1,
		FRAGMENT = 2,
		ALL = FRAGMENT | VERTEX
	};

	enum class Usage
	{
		UNIFORM,
		STORAGE,
	};

	enum Flags
	{
		VARIABLE_DESCRIPTOR = 1,
		PARTIALLY_BOUND = 2,
	};

	struct BindLayoutInfo
	{
		int slot;
		BufferView buffer;
		Stages stage;
		Usage usage;
		Flags flags;
	};

	struct BindSetLayoutInfo
	{
		std::vector<BindLayoutInfo> bindings;
	};

	VkDescriptorSetLayout CreateDescLayout(VkDevice device, BindSetLayoutInfo& createInfo);
	void WriteDescriptorSet(VkDevice device, VkDescriptorSet descSet, BindSetLayoutInfo& writeInfo);
};

