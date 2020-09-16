#ifndef QUEUE_FAMILIES_H
#define QUEUE_FAMILIES_H

#include <optional>
#include <vulkan/vulkan.h>

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily{std::nullopt};
	std::optional<uint32_t> presentFamily{ std::nullopt };
	std::optional<uint32_t> transferFamily{ std::nullopt };

	bool isComplete();
	bool sameIndices();
};

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

#endif