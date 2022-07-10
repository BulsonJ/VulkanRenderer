#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <array>


struct BufferCreateInfo
{
	std::size_t size;

	enum class Usage 
	{
		UNIFORM,
		STORAGE,
		VERTEX,
		INDEX
	} usage;

	enum class Transfer
	{
		NONE,
		SRC,
		DST
	} transfer {Transfer::NONE};
};

template <typename T>
struct Handle {
	uint32_t slot;
};

struct Buffer
{
	VkBuffer buffer;
	VmaAllocation allocation;
	void* ptr;
};

struct BufferView
{
	Handle<Buffer> buffer;
	std::size_t size;
};

class ResourceManager
{
public:
	static ResourceManager* ptr;
	ResourceManager(const VkDevice device, const VmaAllocator allocator) : device(device), allocator(allocator) {}
	~ResourceManager();

	BufferView CreateBuffer(const BufferCreateInfo& createInfo);
	Buffer GetBuffer(const Handle<Buffer>& buffer);
	void DestroyBuffer(const Handle<Buffer>& buffer);
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

