#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

struct Buffer
{
	VkBuffer buffer{ VK_NULL_HANDLE };
	VmaAllocation allocation;
};

