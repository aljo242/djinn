#ifndef IMAGE_INCLUDE_H
#define IMAGE_INCLUDE_H


#include"../ext_inc.h"
#include <vulkan/vulkan.h>
#include "Context.h"
#include "SwapChain.h"


namespace Djinn
{
	void createImage(Context* p_context, SwapChain* p_swapChain, const uint32_t width, const uint32_t height, const uint32_t mipLevels, const VkFormat format,
		const VkSampleCountFlagBits numSamples, const VkImageTiling tiling, const VkImageUsageFlags flags,
		const VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	VkImageView createImageView(Context* p_context, const VkImage image, const VkFormat format, const VkImageAspectFlags aspectFlags, const uint32_t mipLevels);

	struct ImageCreateInfo
	{
		uint32_t width;
		uint32_t height;
		uint32_t mipLevels;
		VkFormat format;
		VkSampleCountFlagBits numSamples;
		VkImageTiling tiling;
		VkImageUsageFlags usageFlags;
		VkMemoryPropertyFlags memoryFlags;
		VkImageAspectFlags aspectFlags;
		VkSharingMode sharingMode;
	};


	class Image
	{
	public:
		Image();
		Image(Context* p_context, const ImageCreateInfo& createInfo);
		void Init(Context* p_context, const ImageCreateInfo& createInfo);
		void CleanUp(Context* p_context);

	public:
		VkImage image					{ VK_NULL_HANDLE };
		VkImageView imageView			{ VK_NULL_HANDLE };
		VkDeviceMemory imageMemory		{ VK_NULL_HANDLE };
	};
}

#endif // IMAGE_INCLUDE_H