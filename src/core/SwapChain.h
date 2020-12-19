#ifndef SWAPCHAIN_INCLUDE_H
#define SWAPCHAIN_INCLUDE_H

#include "../ext_inc.h"

namespace Djinn
{

	class Context;
	class RenderPass;
	class Image;

	class SwapChain
	{
	public:
		SwapChain(Context* p_context);
		void Init(Context* p_context);
		void CleanUp(Context* p_context);
		void createFramebuffers(Context* p_context, VkImageView& colorImageView, VkImageView& depthImageView, VkRenderPass& renderPass);
		void createFramebuffers(Context* p_context, Image* colorImage, Image* depthImage, VkRenderPass& renderPass);


	private:
		void createSwapChainImages(Context* p_context);
		void createSwapChainImageViews(Context* p_context);


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
	SwapChainSupportDetails querySwapChainSupport(Context* p_context);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

	// choose swapchain capabilities
	VkSurfaceFormatKHR chooseSwapChainFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	// TODO change to probably use another present mode
	VkPresentModeKHR chooseSwapChainPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
	// Swap Chain Extent is the resolution of the swap chain buffer image
	VkExtent2D chooseSwapChainExtent(Context* p_context, const VkSurfaceCapabilitiesKHR& capabilities);

}

#endif // SWAPCHAIN_INCLUDE_H