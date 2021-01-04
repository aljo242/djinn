#ifndef COMMANDS_INCLUDE_H
#define COMMANDS_INCLUDE_H

#include <vulkan/vulkan.h>
#include "Context.h"

namespace Djinn
{
	VkCommandBuffer beginSingleTimeCommands(Djinn::Context* p_context, VkCommandPool& commandPool);
	void endSingleTimeCommands(Djinn::Context* p_context, VkCommandPool& commandPool, VkCommandBuffer commandBuffer, VkQueue submitQueue);
}

#endif //COMMANDS_INCLUDE_H