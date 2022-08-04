#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <array>

#include "Graphics/Common.h"
#include "Structures/Slotmap.h"


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

	Slotmap<Buffer> buffers;
	Slotmap<Image> images;
};

