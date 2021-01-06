#include "Buffer.h"
#include "Memory.h"
#include "../DjinnLib/Array.h"

Djinn::Buffer::Buffer(Djinn::Context* p_context, const BufferCreateInfo createInfo)
{
	Init(p_context, createInfo);
}

void Djinn::Buffer::Init(Djinn::Context* p_context, const BufferCreateInfo createInfo)
{
	size = createInfo.size;

	// TODO add functionality for memory pools and custom allocators
	createBuffer(p_context, createInfo.size, createInfo.offset,
		createInfo.usage, createInfo.properties, createInfo.sharingMode,
		buffer, bufferMemory);
}

void Djinn::Buffer::CleanUp(Djinn::Context* p_context)
{
	vkDestroyBuffer(p_context->gpuInfo.device, buffer, nullptr);
	vkFreeMemory(p_context->gpuInfo.device, bufferMemory, nullptr);
}

void Djinn::createBuffer(Djinn::Context* p_context, const VkDeviceSize size, const VkDeviceSize offset,
	VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, const VkSharingMode sharingMode,
	VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	//QueueFamilyIndices queueFamilyIndices{ findQueueFamilies(p_context->physicalDevice, p_context->surface) };
	Djinn::Array1D<uint32_t, 2> queueFamilies{ p_context->queueFamilyIndices.graphicsFamily.value(), p_context->queueFamilyIndices.transferFamily.value() };

	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = usage;
	// currently always want concurrent usage because we want graphics, transfer queues to have access
	bufferCreateInfo.sharingMode = sharingMode;
	bufferCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilies.NumElem());
	bufferCreateInfo.pQueueFamilyIndices = queueFamilies.Ptr();

	auto result{ vkCreateBuffer(p_context->gpuInfo.device, &bufferCreateInfo, nullptr, &buffer) };
	DJINN_VK_ASSERT(result);

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(p_context->gpuInfo.device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(p_context, memRequirements.memoryTypeBits, properties);

	// TODO : make custom allocator that manages this memory and passes offsets
	result = vkAllocateMemory(p_context->gpuInfo.device, &allocInfo, nullptr, &bufferMemory);
	DJINN_VK_ASSERT(result);

	vkBindBufferMemory(p_context->gpuInfo.device, buffer, bufferMemory, offset);
}

void Djinn::copyBuffer(Context* p_context, VkBuffer srcBuffer, VkBuffer dstBuffer, const VkDeviceSize size)
{
	VkCommandBuffer commandBuffer{ beginSingleTimeCommands(p_context, p_context->transferCommandPool) };

	VkBufferCopy copyRegion;
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	endSingleTimeCommands(p_context, p_context->transferCommandPool, commandBuffer, p_context->transferQueue);

}

void Djinn::copyBuffer(Context* p_context, Buffer srcBuffer, Buffer dstBuffer, const VkDeviceSize size)
{
	VkCommandBuffer commandBuffer{ beginSingleTimeCommands(p_context, p_context->transferCommandPool) };

	VkBufferCopy copyRegion;
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer.buffer, dstBuffer.buffer, 1, &copyRegion);

	endSingleTimeCommands(p_context, p_context->transferCommandPool, commandBuffer, p_context->transferQueue);
}

void Djinn::copyDataToMappedBuffer(Djinn::Context* p_context, Djinn::Buffer& stagingBuffer, const VkDeviceSize bufferSize, const VkDeviceSize offset, void* src)
{
	void* dest;
	vkMapMemory(p_context->gpuInfo.device, stagingBuffer.bufferMemory, 0, bufferSize, 0, &dest);
	memcpy(dest, src, static_cast<size_t>(bufferSize));
	vkUnmapMemory(p_context->gpuInfo.device, stagingBuffer.bufferMemory);
}