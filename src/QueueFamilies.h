#ifndef QUEUE_FAMILIES_H
#define QUEUE_FAMILIES_H

#include <optional>
#include <vulkan/vulkan.h>

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily{std::nullopt};
	std::optional<uint32_t> presentFamily{ std::nullopt };

	bool isComplete();
};

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface, VkQueueFlagBits flag);

#endif