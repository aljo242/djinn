#ifndef SWAPCHAIN_INCLUDE_H
#define SWAPCHAIN_INCLUDE_H

#include "../ext_inc.h"

namespace Djinn
{

	class Instance;
	class RenderPass;

	class SwapChain
	{
	public:
		SwapChain(Instance* p_instance);
		void Init(Instance* p_instance);
		void CleanUp(Instance* p_instance);
		void createFramebuffers(Instance* p_instance, VkImageView& colorImageView, VkImageView& depthImageView, VkRenderPass& renderPass);

	private:
		void createSwapChainImages(Instance* p_instance);
		void createSwapChainImageViews(Instance* p_instance);
		//void createFramebuffers(Instance* p_instance, Image* colorImage, Image* depthImage, RenderPass* renderPass);


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

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR			capabilities;
		std::vector<VkSurfaceFormatKHR>		formats;
		std::vector<VkPresentModeKHR>		presentModes;
	};

	// use Physical Device and Surface to query support for swapchains
	SwapChainSupportDetails querySwapChainSupport(Instance* p_instance);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

	// choose swapchain capabilities
	VkSurfaceFormatKHR chooseSwapChainFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	// TODO change to probably use another present mode
	VkPresentModeKHR chooseSwapChainPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
	// Swap Chain Extent is the resolution of the swap chain buffer image
	VkExtent2D chooseSwapChainExtent(Instance* p_instance, const VkSurfaceCapabilitiesKHR& capabilities);

}

#endif // SWAPCHAIN_INCLUDE_H