#include "Image.h"
#include "Memory.h"


Djinn::Image::Image(Context* p_context, const ImageCreateInfo& createInfo)
{
	Init(p_context, createInfo);
}

Djinn::Image::Image() {}

void Djinn::Image::CleanUp(Context* p_context)
{
	vkDestroyImageView(p_context->gpuInfo.device, imageView, nullptr);
	vkDestroyImage(p_context->gpuInfo.device, image, nullptr);
	vkFreeMemory(p_context->gpuInfo.device, imageMemory, nullptr);
}

void Djinn::Image::Init(Context* p_context, const ImageCreateInfo& createInfo)
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

	auto result{ vkCreateImage(p_context->gpuInfo.device, &imageCreateInfo, nullptr, &image) };
	DJINN_VK_ASSERT(result);

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// ALLOCATE IMAGE MEM

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(p_context->gpuInfo.device, image, &memRequirements);

	VkMemoryAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memRequirements.size;
	allocateInfo.memoryTypeIndex = findMemoryType(p_context, memRequirements.memoryTypeBits, createInfo.memoryFlags);

	result = vkAllocateMemory(p_context->gpuInfo.device, &allocateInfo, nullptr, &imageMemory);
	DJINN_VK_ASSERT(result);

	vkBindImageMemory(p_context->gpuInfo.device, image, imageMemory, 0);

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

	result = vkCreateImageView(p_context->gpuInfo.device, &viewCreateInfo, nullptr, &imageView);
	DJINN_VK_ASSERT(result);
}

void Djinn::createImage(Context* p_context, SwapChain* p_swapChain, const uint32_t width, const uint32_t height, const uint32_t mipLevels, const VkFormat format, const VkSampleCountFlagBits numSamples, const VkImageTiling tiling, const VkImageUsageFlags flags, const VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
	VkImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent.width = static_cast<uint32_t>(width);
	imageCreateInfo.extent.height = static_cast<uint32_t>(height);
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = mipLevels;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.format = format;
	imageCreateInfo.tiling = tiling;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.usage = flags;
	imageCreateInfo.sharingMode = p_swapChain->sharingMode;
	imageCreateInfo.samples = numSamples;
	imageCreateInfo.flags = 0; // opt

	auto result{ vkCreateImage(p_context->gpuInfo.device, &imageCreateInfo, nullptr, &image) };
	DJINN_VK_ASSERT(result);

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(p_context->device, image, &memRequirements);

	VkMemoryAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memRequirements.size;
	allocateInfo.memoryTypeIndex = findMemoryType(p_context, memRequirements.memoryTypeBits, properties);

	result = vkAllocateMemory(p_context->gpuInfo.device, &allocateInfo, nullptr, &imageMemory);
	DJINN_VK_ASSERT(result);

	result = vkBindImageMemory(p_context->gpuInfo.device, image, imageMemory, 0);
	DJINN_VK_ASSERT(result);
}

VkImageView Djinn::createImageView(Context* p_context, const VkImage image, const VkFormat format, const VkImageAspectFlags aspectFlags, const uint32_t mipLevels)
{
	VkImageView imageView;

	VkImageViewCreateInfo viewCreateInfo{};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.image = image;
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.format = format;
	viewCreateInfo.subresourceRange.aspectMask = aspectFlags;
	viewCreateInfo.subresourceRange.baseMipLevel = 0;
	viewCreateInfo.subresourceRange.levelCount = mipLevels;
	viewCreateInfo.subresourceRange.baseArrayLayer = 0;
	viewCreateInfo.subresourceRange.layerCount = 1;

	auto result{ vkCreateImageView(p_context->gpuInfo.device, &viewCreateInfo, nullptr, &imageView) };
	DJINN_VK_ASSERT(result);

	return imageView;
}
