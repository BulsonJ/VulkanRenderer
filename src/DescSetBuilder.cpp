#include "DescSetBuilder.h"

#include "VulkanInit.h"

VkDescriptorSetLayout DescriptorSet::CreateDescriptorSetLayout(VkDevice device, LayoutBuildInfo buildInfo)
{
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	for (const auto& bind : buildInfo.bindings)
	{
		VkShaderStageFlags flags{};
		switch (bind.shaderStage)
		{
		case ShaderStage::VERTEX:
			flags = VK_SHADER_STAGE_VERTEX_BIT;
			break;
		case ShaderStage::PIXEL:
			flags = VK_SHADER_STAGE_FRAGMENT_BIT;
			break;
		case ShaderStage::COMPUTE:
			flags = VK_SHADER_STAGE_COMPUTE_BIT;
			break;
		}
		VkDescriptorType type{};
		switch (bind.bufferType)
		{
		case BufferType::UNIFORM:
			type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			break;
		case BufferType::STORAGE:
			type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			break;
		}

		bindings.push_back(VulkanInit::descriptorSetLayoutBinding(type, flags, 0));
	}

	const VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.bindingCount = static_cast<uint32_t>(bindings.size()),
		.pBindings = bindings.data(),
	};

	VkDescriptorSetLayout descriptorSetLayout;
	vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr, &descriptorSetLayout);
	return descriptorSetLayout;
}

void DescriptorSet::WriteDescriptorSet(VkDevice device, WriteInfo writeInfo)
{
	std::vector<VkWriteDescriptorSet> setWrites;
	for (auto& set : writeInfo.descriptorSetWrites)
	{
		VkDescriptorType type{};
		switch (set.bufferType)
		{
		case BufferType::UNIFORM:
			type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			break;
		case BufferType::STORAGE:
			type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			break;
		}

		setWrites.push_back(VulkanInit::writeDescriptorBuffer(type, *(set.descriptorSet), &set.bufferInfo, 0));
	}
	vkUpdateDescriptorSets(device, 3, setWrites.data(), 0, nullptr);
}

