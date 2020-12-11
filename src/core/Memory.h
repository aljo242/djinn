#ifndef MEMORY_INCLUDE_H
#define MEMORY_INCLUDE_H

#include "../ext_inc.h"
#include "Instance.h"

namespace Djinn
{

	uint32_t findMemoryType(Instance* p_instance, const uint32_t typeFilter, const VkMemoryPropertyFlags properties);

}

#endif // MEMORY_INCLUDE_H