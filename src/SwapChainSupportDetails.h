#ifndef SWAP_CHAIN_SUPPORT_DETAIL_H
#define SWAP_CHAIN_SUPPORT_DETAIL_H

#include "ext_inc.h"
#include <vector>

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR			capabilities;
	std::vector<VkSurfaceFormatKHR>		formats;
	std::vector<VkPresentModeKHR>		presentModes;
};

// use Physical Device and Surface to query support for swapchains
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDev, VkSurfaceKHR surface);

// choose swapchain capabilities
VkSurfaceFormatKHR chooseSwapChainFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
// TODO change to probably use another present mode
VkPresentModeKHR chooseSwapChainPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
// Swap Chain Extent is the resolution of the swap chain buffer image
VkExtent2D chooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

#endif