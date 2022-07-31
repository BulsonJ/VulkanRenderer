#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <array>

#include "Graphics/Common.h"


struct BufferCreateInfo
{
	std::size_t size;

	GFX::Buffer::Usage usage {GFX::Buffer::Usage::NONE};

	enum class Transfer
	{
		NONE,
		SRC,
		DST
	} transfer {Transfer::NONE};
};

struct ImageCreateInfo
{
	VkImageCreateInfo imageInfo;
	enum class ImageType
	{
		TEXTURE_2D
	} imageType;
	enum class Usage
	{
		COLOR,
		DEPTH
	} usage;
};

template <typename T>
struct Handle {
	uint32_t slot;
};

struct Buffer
{
	VkBuffer buffer{ VK_NULL_HANDLE };
	VmaAllocation allocation;
	void* ptr;
	std::size_t size;
};

struct Image
{
	VkImage image {VK_NULL_HANDLE};
	VmaAllocation allocation;
	VkImageView imageView;
};

class ResourceManager
{
public:
	static ResourceManager* ptr;
	ResourceManager(const VkDevice device, const VmaAllocator allocator) : device(device), allocator(allocator) {}
	~ResourceManager();

	Handle<Buffer> CreateBuffer(const BufferCreateInfo& createInfo);
	Buffer GetBuffer(const Handle<Buffer>& buffer);
	void DestroyBuffer(const Handle<Buffer>& buffer);

	Handle<Image> CreateImage(const ImageCreateInfo& createInfo);
	Image GetImage(const Handle<Image>& image);
	void DestroyImage(const Handle<Image>& image);
protected:
	const VkDevice device;
	const VmaAllocator allocator;

	std::array<Buffer, 1024> buffers{VK_NULL_HANDLE};

	uint32_t lastHandle{0};
	[[nodiscard]] Handle<Buffer> getNewBufferHandle()
	{
		return Handle<Buffer>{++lastHandle};
	}

	std::array<Image, 1024> images{ VK_NULL_HANDLE };

	uint32_t lastImageHandle{ 0 };
	[[nodiscard]] Handle<Image> getNewImageHandle()
	{
		return Handle<Image>{++lastImageHandle};
	}

};

