#ifndef CHOOSE_CAPABILITIES_H
#define CHOOSE_CAPABILITIES_H

#include "vulkan/vulkan.h"
#include <vector>

VkSurfaceFormatKHR chooseSwapChainFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);


// TODO change to probably use another present mode
VkPresentModeKHR chooseSwapChainPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);

#endif