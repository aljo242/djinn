#ifndef IMAGE_INCLUDE_H
#define IMAGE_INCLUDE_H


#include"../ext_inc.h"
#include <vulkan/vulkan.h>
#include "Instance.h"
#include "SwapChain.h"


namespace Djinn
{
	void createImage(Instance* p_instance, SwapChain* p_swapChain, const uint32_t width, const uint32_t height, const uint32_t mipLevels, const VkFormat format,
		const VkSampleCountFlagBits numSamples, const VkImageTiling tiling, const VkImageUsageFlags flags,
		const VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	VkImageView createImageView(Instance* p_instance, const VkImage image, const VkFormat format, const VkImageAspectFlags aspectFlags, const uint32_t mipLevels);

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
		Image(Instance* p_instance, const ImageCreateInfo& createInfo);
		void Init(Instance* p_instance, const ImageCreateInfo& createInfo);
		void CleanUp(Instance* p_instance);

	private:
		VkImage image					{ VK_NULL_HANDLE };
		VkImageView imageView			{ VK_NULL_HANDLE };
		VkDeviceMemory imageMemory		{ VK_NULL_HANDLE };
	};
}

#endif // IMAGE_INCLUDE_H