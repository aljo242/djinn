#ifndef SWAPCHAIN_INCLUDE_H
#define SWAPCHAIN_INCLUDE_H

#include "../ext_inc.h"

namespace Djinn
{

	class Image;
	class Instance;
	class RenderPass;

	class SwapChain
	{
	public:
		SwapChain(Instance* p_instance);
		void Init(Instance* p_instance);
		void CleanUp(Instance* p_instance);

	private:
		void createSwapChainImages(Instance* p_instance);
		void createFramebuffers(Instance* p_instance, Image* colorImage, Image* depthImage, RenderPass* renderPass);
		void createFramebuffers(Instance* p_instance, std::vector<Image>& images, RenderPass* renderPass);


	public:
		VkSharingMode sharingMode;
		VkSwapchainKHR swapChain{ VK_NULL_HANDLE };
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;
		std::vector<VkImage> swapChainImages;
		std::vector<VkImageView> swapChainImageViews;
		std::vector<VkFramebuffer> swapChainFramebuffers;


	private:

	};

}

#endif // SWAPCHAIN_INCLUDE_H