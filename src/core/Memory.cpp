#include "Memory.h"

uint32_t Djinn::findMemoryType(Context* p_context, const uint32_t typeFilter, const VkMemoryPropertyFlags properties)
{
	const auto memProperties = p_context->gpuInfo.memProperties;

	for (uint32_t i = 0; i <memProperties.memoryTypeCount; ++i)
	{
		const bool memDetect{ (typeFilter & (1 << i)) && ((memProperties.memoryTypes[i].propertyFlags & properties) == properties) };
		if (memDetect)
		{
			return i;
		}
	}

	throw std::runtime_error("Failed to find suitable memory type!");

	return 0;
}