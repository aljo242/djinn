#ifndef MEMORY_INCLUDE_H
#define MEMORY_INCLUDE_H

#include "../ext_inc.h"
#include "Context.h"
#include "../DjinnLib/Types.h"
#include "../DjinnLib/Array.h"
#include "../external/vk_mem_alloc.h"

#include <vector>

namespace Djinn
{
	struct AllocatedBuffer
	{
		VkBuffer buffer;
		VmaAllocation allocation;
	};

	struct AllocatedImage
	{
		VkImage image;
		VmaAllocation allocation;
	};


	enum class MemoryUsage
	{
		MEMORY_USAGE_UNKNOWN,
		MEMORY_USAGE_GPU_ONLY,
		MEMORY_USAGE_CPU_ONLY,
		MEMORY_USAGE_CPU_TO_GPU,
		MEMORY_USAGE_GPU_TO_CPU
	};

	enum class AllocationType
	{
		ALLOCATION_TYPE_FREE,
		ALLOCATION_TYPE_BUFFER,
		ALLOCATION_TYPE_IMAGE,
		ALLOCATION_TYPE_IMAGE_LINEAR,
		ALLOCATION_TYPE_IMAGE_OPTIMAL
	};

	uint32_t findMemoryType(Context* p_context, const uint32_t typeFilter, const VkMemoryPropertyFlags properties);

	class VulkanBlock;

	struct VulkanAllocation_t
	{
		VulkanBlock*		block = nullptr;
		uint32_t			ID = 0;
		VkDeviceMemory		deviceMemory = VK_NULL_HANDLE;
		VkDeviceSize		offset = 0;
		VkDeviceSize		size = 0;
		byte*				data = nullptr;
	};

	class VulkanBlock
	{
		friend class VulkanAllocator;
	public:
	private:
		struct chunk_t
		{
			uint32_t id;
			VkDeviceSize size;
			VkDeviceSize offset;
			chunk_t* prev;
			chunk_t* next;
		};

		chunk_t* head;

		uint32_t nextBlockID;
		uint32_t memoryTypeIndex;
		VkDeviceSize size = 0;
		VkDeviceSize allocated = 0;
		byte* data;

	};

	//using Array1D<std::vector< VulkanBlock* >, 

	class VulkanAllocator
	{

	};
}

#endif // MEMORY_INCLUDE_H