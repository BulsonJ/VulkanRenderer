#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <array>


struct BufferCreateInfo
{
	uint32_t size;

	enum class Usage {
		UNIFORM,
		STORAGE
	};

	Usage usage;
};

template <typename T>
struct Handle {
	uint32_t slot;
};

struct Buffer
{
	VkBuffer buffer;
	VmaAllocation allocation;
};

struct BufferView
{
	Handle<Buffer> buffer;
	uint32_t size;
};

class ResourceManager
{
public:
	static ResourceManager* ptr;
	ResourceManager(const VkDevice device, const VmaAllocator allocator) : device(device), allocator(allocator) {}
	~ResourceManager();

	BufferView CreateBuffer(const BufferCreateInfo& createInfo);
	Buffer GetBuffer(const Handle<Buffer>& buffer);
	void* GetMappedData(Handle<Buffer> handle);
protected:
	const VkDevice device;
	const VmaAllocator allocator;

	std::array<Buffer, 1024> buffers{VK_NULL_HANDLE};

	uint32_t lastHandle{0};
	[[nodiscard]] Handle<Buffer> getNewBufferHandle()
	{
		return Handle<Buffer>{++lastHandle};
	}


};

