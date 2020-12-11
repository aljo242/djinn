#ifndef IMAGE_INCLUDE_H
#define IMAGE_INCLUDE_H


#include"../ext_inc.h"
#include "Instance.h"

namespace Djinn
{

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

		Image(Instance* p_instance, const ImageCreateInfo& createInfo);
		void CleanUp(Instance* p_instance);

	public:
		VkImage image;
		VkImageView imageView;
		VkDeviceMemory imageMemory;
	};
}

#endif // IMAGE_INCLUDE_H