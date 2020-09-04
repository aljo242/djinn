#include "SwapChainSupportDetails.h"

// use Physical Device and Surface to query support for swapchains
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDev, VkSurfaceKHR surface)
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDev, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDev, surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDev, surface, &formatCount, details.formats.data());
	}

	uint32_t presentCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDev, surface, &presentCount, nullptr);

	if (presentCount != 0)
	{
		details.presentModes.resize(presentCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDev, surface, &presentCount, details.presentModes.data());
	}

	return details;
}


VkSurfaceFormatKHR chooseSwapChainFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
	{
		// currently choosing 8-bit color with non-linear sRGB curve
		// https://stackoverflow.com/questions/12524623/what-are-the-practical-differences-when-working-with-colors-in-a-linear-vs-a-no
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	// TODO sort formats for the "next best"
	// need to figure out what i think next best means first

	// default return 
	return availableFormats[0];
}

// TODO change to probably use another present mode
VkPresentModeKHR chooseSwapChainPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes)
	{
		// allows us to create "triple buffering" schemes
		// may want to use immediate
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
	}

	// default return 
	// guaranteed to be present
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
{
	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		return capabilities.currentExtent;
	}
	else
	{
		int width{ 0 };
		int height{ 0 };

		// get the glfw window size and cast to VkExtent object
		glfwGetFramebufferSize(window, &width, &height);
		VkExtent2D actualExtent{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

		// clamp within [capabilities.minextent, capabilities.maxextent]
		actualExtent.width = std::max(capabilities.minImageExtent.width,
			std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height,
			std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}