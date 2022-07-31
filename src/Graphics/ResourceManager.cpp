#include "Graphics/ResourceManager.h"
#include "Graphics/VulkanInit.h"

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

	for (const auto& image : images)
	{
		if (image.image != VK_NULL_HANDLE)
		{
			vkDestroyImageView(device, image.imageView, nullptr);
			vmaDestroyImage(allocator, image.image, image.allocation);
		}
	}
}

Buffer ResourceManager::GetBuffer(const Handle<Buffer>& buffer)
{
	return buffers[buffer.slot];
}

BufferView ResourceManager::CreateBuffer(const BufferCreateInfo& createInfo)
{
	VkBufferCreateInfo bufferInfo = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = createInfo.size,
	};
	switch (createInfo.usage)
	{
	default:
		break;
	case  GFX::Buffer::Usage::NONE:
		break;
	case  GFX::Buffer::Usage::UNIFORM:
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		break;
	case  GFX::Buffer::Usage::STORAGE:
		bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		break;
	case  GFX::Buffer::Usage::VERTEX:
		bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		break;
	case  GFX::Buffer::Usage::INDEX:
		bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		break;
	}

	switch (createInfo.transfer)
	{
	default:
		break;
	case BufferCreateInfo::Transfer::NONE:
		break;
	case BufferCreateInfo::Transfer::SRC:
		bufferInfo.usage = bufferInfo.usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		break;
	case BufferCreateInfo::Transfer::DST:
		bufferInfo.usage = bufferInfo.usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
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

	VmaAllocationInfo allocInfo;
	vmaGetAllocationInfo(allocator, newBuffer.allocation, &allocInfo);
	newBuffer.ptr = allocInfo.pMappedData;

	Handle<Buffer> newHandle = getNewBufferHandle();
	buffers[newHandle.slot] = newBuffer;

	BufferView newView{
		.buffer = newHandle,
		.size = createInfo.size,
	};

	return newView;
}

void ResourceManager::DestroyBuffer(const Handle<Buffer>& buffer)
{
	Buffer& deleteBuffer = buffers[buffer.slot];
	if (deleteBuffer.buffer != VK_NULL_HANDLE)
	{
		vmaDestroyBuffer(allocator, deleteBuffer.buffer, deleteBuffer.allocation);
	}
	deleteBuffer.buffer = VK_NULL_HANDLE;
}

Handle<Image> ResourceManager::CreateImage(const ImageCreateInfo& createInfo)
{
	Image newImage;

	VmaAllocationCreateInfo allocInfo = {
		.usage = VMA_MEMORY_USAGE_AUTO ,
	};

	vmaCreateImage(allocator, &createInfo.imageInfo, &allocInfo, &newImage.image, &newImage.allocation, nullptr);

	const VkImageAspectFlags imageViewType = createInfo.usage == ImageCreateInfo::Usage::COLOR ? VK_IMAGE_ASPECT_COLOR_BIT : VK_IMAGE_ASPECT_DEPTH_BIT;

	VkImageViewCreateInfo imageinfo = {};
	switch (createInfo.imageType)
	{
	case ImageCreateInfo::ImageType::TEXTURE_2D:
		imageinfo = VulkanInit::imageViewCreateInfo(createInfo.imageInfo.format, newImage.image, imageViewType);
		break;
	default:
		break;
	}
	
	switch (createInfo.usage)
	{
	default:
		break;
	case ImageCreateInfo::Usage::DEPTH:
		imageinfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageinfo.components.g = VK_COMPONENT_SWIZZLE_R;
		imageinfo.components.b = VK_COMPONENT_SWIZZLE_R;
		break;
	}

	vkCreateImageView(device, &imageinfo, nullptr, &newImage.imageView);

	Handle<Image> newHandle = getNewImageHandle();
	images[newHandle.slot] = newImage;

	return newHandle;
}

Image ResourceManager::GetImage(const Handle<Image>& image)
{
	return images[image.slot];
}

void ResourceManager::DestroyImage(const Handle<Image>& image)
{
	Image& deleteImage = images[image.slot];
	if (deleteImage.image != VK_NULL_HANDLE)
	{
		vmaDestroyImage(allocator, deleteImage.image, deleteImage.allocation);
		vkDestroyImageView(device, deleteImage.imageView, nullptr);
	}
	deleteImage.image = VK_NULL_HANDLE;
}


