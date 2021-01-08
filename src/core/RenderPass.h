#ifndef RENDERPASS_INCLUDE_H
#define RENDERPASS_INCLUDE_H

#include "../ext_inc.h"

namespace Djinn
{
	class Context;

	struct RenderPassConfig
	{
		VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
		VkFormat swapChainFormat;
		VkFormat depthFormat;
	};

	class RenderPass
	{
	public:
		RenderPass() = default;
		RenderPass(Djinn::Context* p_context, const RenderPassConfig& config);
		void Init(Djinn::Context* p_context, const RenderPassConfig& config);
		void CleanUp(Djinn::Context* p_context);


	public:
		VkRenderPass handle{ VK_NULL_HANDLE };
	};
}

#endif // RENDERPASS_INCLUDE_H