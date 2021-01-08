#ifndef RENDERPASS_INCLUDE_H
#define RENDERPASS_INCLUDE_H

#include "../ext_inc.h"

namespace Djinn
{
	struct RenderPassConfig
	{
		VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
		VkFormat swapChainFormat;
		VkFormat depthFormat;
	};

	class RenderPass
	{
	public:

	private:


	public:
		VkRenderPass renderPass{ VK_NULL_HANDLE };

	private:
	};
}

#endif // RENDERPASS_INCLUDE_H