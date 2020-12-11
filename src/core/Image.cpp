#include "Image.h"
#include "Memory.h"


Djinn::Image::Image(Instance* p_instance, const ImageCreateInfo& createInfo)
{
	VkImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent.width = static_cast<uint32_t>(createInfo.width);
	imageCreateInfo.extent.height = static_cast<uint32_t>(createInfo.height);
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = createInfo.mipLevels;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.format = createInfo.format;
	imageCreateInfo.tiling = createInfo.tiling;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.usage = createInfo.usageFlags;
	imageCreateInfo.sharingMode = createInfo.sharingMode;
	imageCreateInfo.samples = createInfo.numSamples;
	imageCreateInfo.flags = 0; // opt

	auto result{ vkCreateImage(p_instance->device, &imageCreateInfo, nullptr, &image) };
	DJINN_VK_ASSERT(result);

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// ALLOCATE IMAGE MEM

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(p_instance->device, image, &memRequirements);

	VkMemoryAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memRequirements.size;
	allocateInfo.memoryTypeIndex = findMemoryType(p_instance, memRequirements.memoryTypeBits, createInfo.memoryFlags);

	result = vkAllocateMemory(p_instance->device, &allocateInfo, nullptr, &imageMemory);
	DJINN_VK_ASSERT(result);

	vkBindImageMemory(p_instance->device, image, imageMemory, 0);

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// CREATE IMAGE VIEW

	VkImageViewCreateInfo viewCreateInfo{};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.image = image;
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.format = createInfo.format;
	viewCreateInfo.subresourceRange.aspectMask = createInfo.aspectFlags;
	viewCreateInfo.subresourceRange.baseMipLevel = 0;
	viewCreateInfo.subresourceRange.levelCount = createInfo.mipLevels;
	viewCreateInfo.subresourceRange.baseArrayLayer = 0;
	viewCreateInfo.subresourceRange.layerCount = 1;

	result = vkCreateImageView(p_instance->device, &viewCreateInfo, nullptr, &imageView);
	DJINN_VK_ASSERT(result);
}

void Djinn::Image::CleanUp(Instance* p_instance)
{
	vkDestroyImageView(p_instance->device, imageView, nullptr);
	vkDestroyImage(p_instance->device, image, nullptr);
	vkFreeMemory(p_instance->device, imageMemory, nullptr);
}
