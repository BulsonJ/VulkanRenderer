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

	struct BindLayoutCreateInfo
	{
		int slot;
		Stages stage;
		Usage usage;
	};

	struct SetBindLayoutCreateInfo
	{
		std::vector<BindLayoutCreateInfo> bindings;
	};

	VkDescriptorSetLayout CreateDescLayout(VkDevice device, SetBindLayoutCreateInfo& createInfo);
	
	struct BindWriteInfo
	{
		int slot;
		Usage usage;
		BufferView buffer;
	};

	struct SetBindWriteInfo
	{
		std::vector<BindWriteInfo> writes;
	};

	void WriteDescriptorSet(VkDevice device, VkDescriptorSet descSet, SetBindWriteInfo& writeInfo);
};

