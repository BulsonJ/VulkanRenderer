#include "DescriptorSet.h"
#include <cstdint>

#include "VulkanInit.h"

VkDescriptorSetLayout Desc::CreateDescLayout(VkDevice device, SetBindLayoutCreateInfo& createInfo)
{
	VkDescriptorSetLayout setLayout;

	std::vector<VkDescriptorSetLayoutBinding> bindings;
	for (const auto& bind : createInfo.bindings)
	{
		VkDescriptorSetLayoutBinding setBind = {
			.binding = static_cast<uint32_t>(bind.slot),
			.descriptorCount = 1,
			.pImmutableSamplers = nullptr,
		};

		if ((bind.stage && Desc::Stages::VERTEX) != 0) 
		{
			setBind.stageFlags = setBind.stageFlags | VK_SHADER_STAGE_VERTEX_BIT; 
		}
		if ((bind.stage && Desc::Stages::FRAGMENT) != 0)
		{
			setBind.stageFlags = setBind.stageFlags | VK_SHADER_STAGE_FRAGMENT_BIT;
		}

		switch (bind.usage)
		{
		case Usage::STORAGE:
			setBind.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			break;
		case Usage::UNIFORM:
			setBind.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			break;
		}

		bindings.emplace_back(setBind);
	}

	VkDescriptorSetLayoutCreateInfo descSetCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = static_cast<uint32_t>(bindings.size()),
		.pBindings = bindings.data(),
	};

	vkCreateDescriptorSetLayout(device, &descSetCreateInfo, nullptr, &setLayout);

	return setLayout;
}

void Desc::WriteDescriptorSet(VkDevice device, VkDescriptorSet descSet, SetBindWriteInfo& writeInfo)
{
	std::vector<VkWriteDescriptorSet> writes;
	for (const auto& write : writeInfo.writes)
	{
		VkDescriptorBufferInfo bufferInfo{
			.buffer = ResourceManager::ptr->GetBuffer(write.buffer.buffer).buffer,
			.offset = 0,
			.range = write.buffer.size,
		};

		VkDescriptorType descType;
		switch (write.usage)
		{
		case Usage::STORAGE:
			descType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			break;
		case Usage::UNIFORM:
			descType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			break;
		}

		const VkWriteDescriptorSet writeSet = VulkanInit::writeDescriptorBuffer(descType, descSet, &bufferInfo, write.slot);

		writes.push_back(writeSet);
	}

	vkUpdateDescriptorSets(device, writes.size(), writes.data(), 0, nullptr);
}
