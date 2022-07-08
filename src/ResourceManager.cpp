#include "ResourceManager.h"

ResourceManager* ResourceManager::ptr = nullptr;


ResourceManager::~ResourceManager()
{
	for (const auto& buffer : buffers)
	{
		if (buffer.buffer != VK_NULL_HANDLE)
		{
			vmaDestroyBuffer(allocator, buffer.buffer, buffer.allocation);
		}
	}
}

Buffer ResourceManager::GetBuffer(const Handle<Buffer>& buffer)
{
	return buffers[buffer.slot];
}

Handle<Buffer> ResourceManager::CreateBuffer(const BufferCreateInfo& createInfo)
{
	VkBufferCreateInfo bufferInfo = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = createInfo.size,
	};
	switch (createInfo.usage)
	{
	default:
		break;
	case BufferCreateInfo::Usage::UNIFORM:
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		break;
	case BufferCreateInfo::Usage::STORAGE:
		bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		break;
	}

	VmaAllocationCreateInfo vmaallocInfo = {
		.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
		.usage = VMA_MEMORY_USAGE_AUTO,
	};

	Buffer newBuffer;

	//allocate the buffer
	vmaCreateBuffer(allocator, &bufferInfo, &vmaallocInfo,
		&newBuffer.buffer,
		&newBuffer.allocation,
		nullptr);

	Handle<Buffer> newHandle = getNewBufferHandle();
	buffers[newHandle.slot] = newBuffer;

	return newHandle;
}
