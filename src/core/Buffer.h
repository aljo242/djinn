#ifndef BUFFER_INCLUDE_H
#define BUFFER_INCLUDE_H

#include <vulkan/vulkan.h>
#include "Context.h"
#include "SwapChain.h"
#include "Commands.h"

namespace Djinn
{
	struct BufferCreateInfo
	{
		VkDeviceSize size{ 0 };
		VkDeviceSize offset {0};
		VkBufferUsageFlags usage{0};
		VkMemoryPropertyFlags properties{ 0 };
		VkSharingMode sharingMode{ VK_SHARING_MODE_CONCURRENT };
	};

	class Buffer
	{
	public:
		Buffer() = default;
		Buffer(Djinn::Context* p_context, const BufferCreateInfo createInfo);

		void Init(Djinn::Context* p_context, const BufferCreateInfo createInfo);
		void CleanUp(Djinn::Context* p_context);

		VkBuffer buffer {VK_NULL_HANDLE};
		VkDeviceMemory bufferMemory{VK_NULL_HANDLE};
		VkDeviceSize size{ 0 };
	};

	void createBuffer(Djinn::Context* p_context, const VkDeviceSize size, const VkDeviceSize offset,
		VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, const VkSharingMode sharingMode,
		VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	void copyBuffer(Djinn::Context* p_context, VkBuffer srcBuffer, VkBuffer dstBuffer, const VkDeviceSize size);
	void copyBuffer(Djinn::Context* p_context, Djinn::Buffer srcBuffer, Djinn::Buffer dstBuffer, const VkDeviceSize size);
	void copyToMappedBuffer(Djinn::Context* p_context, Djinn::Buffer& stagingBuffer, const VkDeviceSize bufferSize, const VkDeviceSize offset, void* src);

}

#endif //BUFFER_INCLUDE_H