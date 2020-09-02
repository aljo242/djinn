#include "QueueFamilies.h"
#include <vector>

bool QueueFamilyIndices::isComplete()
{
	return graphicsFamily.has_value() && presentFamily.has_value();
}


QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface, VkQueueFlagBits flag)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount{ 0 };
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	uint32_t i{ 0 };
	for (const auto& queueFamily : queueFamilies)
	{
		VkBool32 presentSupport{ VK_FALSE };
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		if (presentSupport)
		{
			indices.presentFamily.emplace(i);
		}

		// mark if the queue matches the requested tpe
		if (queueFamily.queueFlags & flag)
		{
			indices.graphicsFamily.emplace(i);
		}

		if (indices.isComplete())
		{
			break;
		}

		++i;
	}


	return indices;
}