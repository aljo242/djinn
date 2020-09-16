#include "QueueFamilies.h"
#include <vector>

bool QueueFamilyIndices::isComplete()
{
	return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
}

bool QueueFamilyIndices::sameIndices()
{
	if (this->isComplete())
	{
		return graphicsFamily.value() == presentFamily.value();
	}

	return false;
}



QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount{ 0 };
	vkGetPhysicalDeviceQueueFamilyProperties2(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties2> queueFamilies(queueFamilyCount);
	for (auto& queueFamily : queueFamilies)
	{
		queueFamily.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
		// TODO use pNext to query extra information
		queueFamily.pNext = nullptr;
	}
	vkGetPhysicalDeviceQueueFamilyProperties2(device, &queueFamilyCount, queueFamilies.data());

	uint32_t i{ 0 };
	for (const auto& queueFamily : queueFamilies)
	{
		VkBool32 presentSupport{ VK_FALSE };
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		if (presentSupport)
		{
			indices.presentFamily.emplace(i);
		}

		// mark if the queue matches the requested type
		if (queueFamily.queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily.emplace(i);
		}

		if ((queueFamily.queueFamilyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT)
			&&
			!(queueFamily.queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT))
		{
			indices.transferFamily.emplace(i);
		}

		if (indices.isComplete())
		{

			break;
		}

		++i;
	}


	return indices;
}