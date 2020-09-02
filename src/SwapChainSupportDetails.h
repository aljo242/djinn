#ifndef SWAP_CHAIN_SUPPORT_DETAIL_H
#define SWAP_CHAIN_SUPPORT_DETAIL_H

#include <vector>

#include <vulkan/vulkan.h>

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR			capabilities;
	std::vector<VkSurfaceFormatKHR>		formats;
	std::vector<VkPresentModeKHR>		presentModes;
};

// use Physical Device and Surface to query support for swapchains
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDev, VkSurfaceKHR surface);
#endif