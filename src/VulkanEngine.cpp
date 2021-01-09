#include "VulkanEngine.h"

#include "core/Context.h"
#include "core/SwapChain.h"
#include "core/Image.h"
#include "core/defs.h"
#include "core/Memory.h"
#include "core/Commands.h"

#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include <tiny_obj_loader.h>


Djinn::VulkanEngine::~VulkanEngine()
{
	delete p_swapChain;
	delete p_context;
}

void Djinn::VulkanEngine::Init()
{
	initVulkan();
}


bool Djinn::VulkanEngine::WindowOpen()
{
	return !glfwWindowShouldClose(p_context->window);
}

void Djinn::VulkanEngine::QueryWindowEvents()
{
	glfwPollEvents();
}

void Djinn::VulkanEngine::initVulkan()
{
	p_context = new Context();
	p_context->Init();
	const auto indices = p_context->queueFamilyIndices;
	msaaSamples = p_context->renderConfig.msaaSamples;
	p_swapChain = new SwapChain(p_context);

	createRenderPass();			//
	createDescriptorSetLayout();//
	createGraphicsPipeline();	//
	createColorResources();     //
	createDepthResources();		//
	//createFramebuffers();		//
	p_swapChain->createFramebuffers(p_context, &colorImage, &depthImage, renderPass);
	//createCommandPool();		//
	createTextureImage();		// 
	createTextureImageView();	//
	createTextureSampler();		//
	loadModel(MODEL_PATH);				//
	createVertexBufferStaged();		//
	createIndexBufferStaged();		//
	createUniformBuffers();		//
	createDescriptorPool();		//
	createDescriptorSets();		//
	createCommandBuffers();		//
	createSyncObjects();		//
}

void Djinn::VulkanEngine::CleanUp()
{
	// wait for the device to not be "mid-work" before we destroy objects
	vkDeviceWaitIdle(p_context->gpuInfo.device);

	//cleanupSwapChain();
	vkDestroyPipelineLayout(p_context->gpuInfo.device, graphicsPipeline.pipelineLayout, nullptr);
	vkDestroyPipeline(p_context->gpuInfo.device, graphicsPipeline.pipeline, nullptr);
	renderPass.CleanUp(p_context);

	for (auto& buffer : _uniformBuffers)
	{
		buffer.CleanUp(p_context);
	}

	colorImage.CleanUp(p_context);
	depthImage.CleanUp(p_context);

	vkDestroyDescriptorPool(p_context->gpuInfo.device, descriptorPool, nullptr);

	vkDestroySampler(p_context->gpuInfo.device, textureSampler, nullptr);
	vkDestroyImageView(p_context->gpuInfo.device, textureImageView, nullptr);

	vkDestroyDescriptorSetLayout(p_context->gpuInfo.device, descriptorSetLayout, nullptr);


	_vertexBuffer.CleanUp(p_context);
	_indexBuffer.CleanUp(p_context);

	vkDestroyImage(p_context->gpuInfo.device, textureImage, nullptr);
	vkFreeMemory(p_context->gpuInfo.device, textureImageMemory, nullptr);

	p_swapChain->CleanUp(p_context);


	// destroy sync objects
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroySemaphore(p_context->gpuInfo.device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(p_context->gpuInfo.device, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(p_context->gpuInfo.device, inFlightFences[i], nullptr);
	}



	p_context->CleanUp();
}



void Djinn::VulkanEngine::cleanupSwapChain()
{
	p_swapChain->CleanUp(p_context);

	vkFreeCommandBuffers(p_context->gpuInfo.device, p_context->graphicsCommandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
	vkDestroyPipeline(p_context->gpuInfo.device, graphicsPipeline.pipeline, nullptr);
	vkDestroyPipelineLayout(p_context->gpuInfo.device, graphicsPipeline.pipelineLayout, nullptr);
	renderPass.CleanUp(p_context);

	colorImage.CleanUp(p_context);
	depthImage.CleanUp(p_context);

	vkDestroyDescriptorPool(p_context->gpuInfo.device, descriptorPool, nullptr);

}

void Djinn::VulkanEngine::recreateSwapChain()
{
	// check the size 
	p_context->queryWindowSize();

	vkDeviceWaitIdle(p_context->gpuInfo.device);

	cleanupSwapChain();

	// re-init
	p_swapChain->Init(p_context);
	createRenderPass();
	createGraphicsPipeline();
	createColorResources();
	createDepthResources();
	p_swapChain->createFramebuffers(p_context, &colorImage, &depthImage, renderPass);
	createDescriptorPool();		//
	createDescriptorSets();		//
	createCommandBuffers();
}


void Djinn::VulkanEngine::createRenderPass()
{
	RenderPassConfig config;
	config.msaaSamples = msaaSamples;
	config.swapChainFormat = p_swapChain->swapChainImageFormat;
	config.depthFormat = findDepthFormat(p_context);

	renderPass.Init(p_context, config);
}

void Djinn::VulkanEngine::createGraphicsPipeline()
{
	ShaderLoader vertShader("shader/vert.spv", p_context->gpuInfo.device, VK_SHADER_STAGE_VERTEX_BIT);
	ShaderLoader fragShader("shader/frag.spv", p_context->gpuInfo.device, VK_SHADER_STAGE_FRAGMENT_BIT);

	GraphicsPipelineBuilder graphicsPipelineBuilder;
	PipelineConfig pipelineConfig;
	pipelineConfig.descriptorSetLayouts.push_back(descriptorSetLayout);
	pipelineConfig.msaaSamples = msaaSamples;
	pipelineConfig.polygonMode = VK_POLYGON_MODE_LINE;
	pipelineConfig.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipelineConfig.renderPass = renderPass.handle;
	pipelineConfig.shaderLoaders.push_back(vertShader);
	pipelineConfig.shaderLoaders.push_back(fragShader);

	graphicsPipeline = graphicsPipelineBuilder.BuildPipeline(p_context, p_swapChain, pipelineConfig);

	// cleanup
	vertShader.DestroyModule();
	fragShader.DestroyModule();
}


void Djinn::VulkanEngine::createDepthResources()
{
	const auto depthFormat{ findDepthFormat(p_context) };

	ImageCreateInfo createInfo{};
	createInfo.width = p_swapChain->swapChainExtent.width;
	createInfo.height = p_swapChain->swapChainExtent.height;
	createInfo.mipLevels = 1;
	createInfo.format = depthFormat;
	createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	createInfo.memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	createInfo.usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	createInfo.aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
	createInfo.sharingMode = p_swapChain->sharingMode;
	createInfo.numSamples = msaaSamples;

	depthImage.Init(p_context, createInfo);
}

void Djinn::VulkanEngine::createTextureImage()
{
	int texWidth{ 0 };
	int texHeight{ 0 };
	int texChannels{ 0 };

	stbi_uc* pixels{ stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha) };
	assert(pixels != nullptr);
	const VkDeviceSize imageSize = texWidth * texHeight * 4;

	m_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, 0);

	void* data;
	vkMapMemory(p_context->gpuInfo.device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(p_context->gpuInfo.device, stagingBufferMemory);

	stbi_image_free(pixels);

	createImage(texWidth, texHeight, m_mipLevels, VK_FORMAT_R8G8B8A8_SRGB, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

	// transition image to transfer dst -> copy to image -> transfer to shader read only to sample from
	transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_mipLevels);
	copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
	//transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_mipLevels);

	vkDestroyBuffer(p_context->gpuInfo.device, stagingBuffer, nullptr);
	vkFreeMemory(p_context->gpuInfo.device, stagingBufferMemory, nullptr);

	generateMipMaps(textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, m_mipLevels);

}

VkImageView Djinn::VulkanEngine::createImageView(const VkImage image, const VkFormat format, const VkImageAspectFlags aspectFlags, const uint32_t mipLevels)
{
	VkImageView imageView;

	VkImageViewCreateInfo viewCreateInfo{};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.image = image;
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.format = format;
	viewCreateInfo.subresourceRange.aspectMask = aspectFlags;
	viewCreateInfo.subresourceRange.baseMipLevel = 0;
	viewCreateInfo.subresourceRange.levelCount = mipLevels;
	viewCreateInfo.subresourceRange.baseArrayLayer = 0;
	viewCreateInfo.subresourceRange.layerCount = 1;

	auto result{ vkCreateImageView(p_context->gpuInfo.device, &viewCreateInfo, nullptr, &imageView) };
	DJINN_VK_ASSERT(result);

	return imageView;
}

void Djinn::VulkanEngine::createTextureImageView()
{
	textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, m_mipLevels);
}


void Djinn::VulkanEngine::createTextureSampler()
{
	VkSamplerCreateInfo samplerCreateInfo{};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.anisotropyEnable = VK_TRUE;
	samplerCreateInfo.maxAnisotropy = 16.0f;
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
	samplerCreateInfo.compareEnable = VK_FALSE;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = static_cast<float>(m_mipLevels);

	auto result{ vkCreateSampler(p_context->gpuInfo.device, &samplerCreateInfo, nullptr, &textureSampler) };
	DJINN_VK_ASSERT(result);
}


void Djinn::VulkanEngine::createColorResources()
{
	const auto colorFormat = p_swapChain->swapChainImageFormat;

	ImageCreateInfo createInfo{};
	createInfo.width = p_swapChain->swapChainExtent.width;
	createInfo.height = p_swapChain->swapChainExtent.height;
	createInfo.mipLevels = 1;
	createInfo.format = colorFormat;
	createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	createInfo.memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	createInfo.usageFlags = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	createInfo.sharingMode = p_swapChain->sharingMode;
	createInfo.numSamples = msaaSamples;

	colorImage.Init(p_context, createInfo);

}

VkCommandBuffer Djinn::VulkanEngine::beginSingleTimeCommands(VkCommandPool& commandPool)
{
	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandPool = commandPool;
	allocateInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(p_context->gpuInfo.device, &allocateInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void Djinn::VulkanEngine::endSingleTimeCommands(VkCommandPool& commandPool, VkCommandBuffer commandBuffer, VkQueue submitQueue)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(submitQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(submitQueue);

	vkFreeCommandBuffers(p_context->gpuInfo.device, commandPool, 1, &commandBuffer);
}

// transition image layout by inserting image memory barrier to commandBuffer
void Djinn::VulkanEngine::transitionImageLayout(const VkImage image, const VkFormat format, const VkImageLayout oldLayout,
	const VkImageLayout newLayout, const uint32_t mipLevels)
{
	VkCommandBuffer commandBuffer{ beginSingleTimeCommands(p_context->transferCommandPool) };

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = 0; // TODO
	barrier.dstAccessMask = 0; // TODO

	VkPipelineStageFlags srcFlags;
	VkPipelineStageFlags dstFlags;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		srcFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		srcFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		throw std::runtime_error("Unsupported layout transition!");
	}

	vkCmdPipelineBarrier(commandBuffer, srcFlags, dstFlags, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	endSingleTimeCommands(p_context->transferCommandPool, commandBuffer, p_context->transferQueue);
}


void Djinn::VulkanEngine::copyBufferToImage(VkBuffer buffer, VkImage image, const uint32_t width, const uint32_t height)
{
	VkCommandBuffer commandBuffer{ beginSingleTimeCommands(p_context->transferCommandPool) };

	// copy the whole thing [0, 0, 0] -> [width, height, 1]
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { width, height, 1 };

	// currently assumes layout has been transistioned to this state
	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	endSingleTimeCommands(p_context->transferCommandPool, commandBuffer, p_context->transferQueue);
}


void Djinn::VulkanEngine::generateMipMaps(VkImage image, const VkFormat format, const uint32_t texWidth, const uint32_t texHeight, const uint32_t mipLevels)
{
	// check if image formats support linear blitting
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(p_context->gpuInfo.gpu, format, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
	{
		throw std::runtime_error("texture image format does not support linear blitting!");
	}

	VkCommandPool commandPool = p_context->graphicsCommandPool;
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(commandPool);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	auto mipWidth = static_cast<int32_t>(texWidth);
	auto mipHeight = static_cast<int32_t>(texHeight);

	for (uint32_t i = 1; i < m_mipLevels; ++i)
	{
		const int32_t currentMipLevel = i - 1;
		const int32_t nextMiplevel = i;
		barrier.subresourceRange.baseMipLevel = currentMipLevel;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		// we are actually specifying how the mip is downsampled
		// we are a
		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };

		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = currentMipLevel;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;

		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1,
			mipHeight > 1 ? mipHeight / 2 : 1,
			1 };

		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = nextMiplevel;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(commandBuffer,
			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit, VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		if (mipWidth > 1)
		{
			mipWidth /= 2;
		}
		if (mipHeight > 1)
		{
			mipHeight /= 2;
		}
	}

	// handle last barrier
	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	endSingleTimeCommands(commandPool, commandBuffer, p_context->graphicsQueue);

}

void Djinn::VulkanEngine::createImage(const uint32_t width, const uint32_t height, const uint32_t mipLevels, const VkFormat format,
	const VkSampleCountFlagBits numSamples, const VkImageTiling tiling, const VkImageUsageFlags flags,
	const VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
	VkImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent.width = static_cast<uint32_t>(width);
	imageCreateInfo.extent.height = static_cast<uint32_t>(height);
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = mipLevels;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.format = format;
	imageCreateInfo.tiling = tiling;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.usage = flags;
	imageCreateInfo.sharingMode = p_swapChain->sharingMode;
	imageCreateInfo.samples = numSamples;
	imageCreateInfo.flags = 0; // opt

	auto result{ vkCreateImage(p_context->gpuInfo.device, &imageCreateInfo, nullptr, &image) };
	DJINN_VK_ASSERT(result);

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(p_context->gpuInfo.device, image, &memRequirements);

	VkMemoryAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memRequirements.size;
	allocateInfo.memoryTypeIndex = findMemoryType(p_context, memRequirements.memoryTypeBits, properties);

	result = vkAllocateMemory(p_context->gpuInfo.device, &allocateInfo, nullptr, &imageMemory);
	DJINN_VK_ASSERT(result);

	result = vkBindImageMemory(p_context->gpuInfo.device, image, imageMemory, 0);
	DJINN_VK_ASSERT(result);
}


void Djinn::VulkanEngine::loadModel(const std::string& path)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err;
	//const char* path = MODEL_PATH.c_str();

	const auto ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, path.c_str());

	if (!err.empty())
	{
		spdlog::error(err);
	}

	assert(ret);

	std::unordered_map<Vertex, uint32_t> uniqueVertices{};

	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			Vertex vertex{};

			vertex.position =
			{
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.texCoord =
			{
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.color = { 1.0f, 1.0f, 1.0f };

			if (uniqueVertices.count(vertex) == 0)
			{
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}

			vertexIndices.push_back(uniqueVertices[vertex]);
		}
	}
}


void Djinn::VulkanEngine::createDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.pImmutableSamplers = nullptr; // opt
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding samplerLayourBinding{};
	samplerLayourBinding.binding = 1;
	samplerLayourBinding.descriptorCount = 1;
	samplerLayourBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayourBinding.pImmutableSamplers = nullptr;
	samplerLayourBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	Djinn::Array1D<VkDescriptorSetLayoutBinding, 2> bindings{ uboLayoutBinding, samplerLayourBinding };

	VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.NumElem());
	layoutCreateInfo.pBindings = bindings.Ptr();

	auto result{ (vkCreateDescriptorSetLayout(p_context->gpuInfo.device, &layoutCreateInfo, nullptr, &descriptorSetLayout)) };
	DJINN_VK_ASSERT(result);
}


void Djinn::VulkanEngine::createVertexBufferStaged()
{
	const VkDeviceSize bufferSize{ sizeof(vertices[0]) * vertices.size() };

	BufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.size = bufferSize;
	bufferCreateInfo.offset = 0;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCreateInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	bufferCreateInfo.sharingMode = p_swapChain->sharingMode;

	Buffer _stagingBuffer;
	_stagingBuffer.Init(p_context, bufferCreateInfo);

	bufferCreateInfo.size = bufferSize;
	bufferCreateInfo.offset = 0;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferCreateInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	bufferCreateInfo.sharingMode = p_swapChain->sharingMode;
	_vertexBuffer.Init(p_context, bufferCreateInfo);

	copyDataToMappedBuffer(p_context, _stagingBuffer, bufferSize, 0, vertices.data());
	copyBuffer(p_context, _stagingBuffer, _vertexBuffer, bufferSize);

	_stagingBuffer.CleanUp(p_context);

}



void Djinn::VulkanEngine::createIndexBufferStaged()
{
	const VkDeviceSize bufferSize{ sizeof(vertexIndices[0]) * vertexIndices.size() };

	BufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.size = bufferSize;
	bufferCreateInfo.offset = 0;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCreateInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	bufferCreateInfo.sharingMode = p_swapChain->sharingMode;

	Buffer _stagingBuffer;
	_stagingBuffer.Init(p_context, bufferCreateInfo);

	bufferCreateInfo.size = bufferSize;
	bufferCreateInfo.offset = 0;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	bufferCreateInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	bufferCreateInfo.sharingMode = p_swapChain->sharingMode;
	_indexBuffer.Init(p_context, bufferCreateInfo);

	copyDataToMappedBuffer(p_context, _stagingBuffer, bufferSize, 0, vertexIndices.data());
	copyBuffer(p_context, _stagingBuffer, _indexBuffer, bufferSize);

	_stagingBuffer.CleanUp(p_context);

}

void Djinn::VulkanEngine::createUniformBuffers()
{
	constexpr VkDeviceSize bufferSize{ sizeof(UniformBufferObject) };
	const size_t swapchainSize{ p_swapChain->swapChainImages.size() };


	_uniformBuffers.resize(swapchainSize);
	//uniformBuffersMemory.resize(swapchainSize);

	BufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.size = bufferSize;
	bufferCreateInfo.offset = 0;
	bufferCreateInfo.sharingMode = p_swapChain->sharingMode;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	bufferCreateInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	for (auto& buffer : _uniformBuffers)
	{
		buffer.Init(p_context, bufferCreateInfo);
	}
}

void Djinn::VulkanEngine::createDescriptorPool()
{
	Array1D<VkDescriptorPoolSize, 2> poolSizes;
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(p_swapChain->swapChainImages.size());
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(p_swapChain->swapChainImages.size());

	VkDescriptorPoolCreateInfo poolCreateInfo{};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.NumElem());
	poolCreateInfo.pPoolSizes = poolSizes.Ptr();
	poolCreateInfo.maxSets = static_cast<uint32_t>(p_swapChain->swapChainImages.size());

	auto result{ (vkCreateDescriptorPool(p_context->gpuInfo.device, &poolCreateInfo, nullptr, &descriptorPool)) };
	DJINN_VK_ASSERT(result);
}


void Djinn::VulkanEngine::createDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(p_swapChain->swapChainImages.size(), descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(p_swapChain->swapChainImages.size());
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(p_swapChain->swapChainImages.size());
	auto result{ (vkAllocateDescriptorSets(p_context->gpuInfo.device, &allocInfo, descriptorSets.data())) };
	DJINN_VK_ASSERT(result);

	for (size_t i = 0; i < p_swapChain->swapChainImages.size(); ++i)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = _uniformBuffers[i].buffer;
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureImageView;
		imageInfo.sampler = textureSampler;

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
		//Djinn::Array1D<VkWriteDescriptorSet, 2> descriptorWrites;
		//VkWriteDescriptorSet descriptorWrites[2];

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;
		descriptorWrites[0].pImageInfo = nullptr; // opt
		descriptorWrites[0].pTexelBufferView = nullptr; // opt

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pBufferInfo = nullptr; //opt
		descriptorWrites[1].pImageInfo = &imageInfo;
		descriptorWrites[1].pTexelBufferView = nullptr; // opt

		vkUpdateDescriptorSets(p_context->gpuInfo.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void Djinn::VulkanEngine::updateUniformBuffer(const uint32_t imageIndex)
{
	static auto startTime{ std::chrono::high_resolution_clock::now() };
	const auto currentTime{ std::chrono::high_resolution_clock::now() };

	const float elapsedTime{ std::chrono::duration<float, std::chrono::seconds::period>
														(currentTime - startTime).count() };

	UniformBufferObject ubo{};
	ubo.model = glm::rotate(glm::mat4(1.0f), elapsedTime * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.projection = glm::perspective(glm::radians(45.0f), (static_cast<float>(p_swapChain->swapChainExtent.width) / static_cast<float>(p_swapChain->swapChainExtent.height)), 0.1f, 10.0f);
	ubo.projection[1][1] *= -1.0f;

	copyDataToMappedBuffer(p_context, _uniformBuffers[imageIndex], sizeof(ubo), 0, &ubo);
}


void Djinn::VulkanEngine::createBuffer(const VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
	VkBuffer& buffer, VkDeviceMemory& bufferMemory, const VkDeviceSize offset)
{
	//QueueFamilyIndices queueFamilyIndices{ findQueueFamilies(p_context->physicalDevice, p_context->surface) };
	Array1D<uint32_t, 2> queueFamilies{ p_context->queueFamilyIndices.graphicsFamily.value(), p_context->queueFamilyIndices.transferFamily.value() };

	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = usage;
	// currently always want concurrent usage because we want graphics, transfer queues to have access
	bufferCreateInfo.sharingMode = p_swapChain->sharingMode;
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


void Djinn::VulkanEngine::createCommandBuffers()
{
	// create clear values
	Array1D<VkClearValue, 2> clearValues{};
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };

	commandBuffers.resize(p_swapChain->swapChainFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = p_context->graphicsCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

	auto result{ vkAllocateCommandBuffers(p_context->gpuInfo.device, &allocInfo, commandBuffers.data()) };
	DJINN_VK_ASSERT(result);

	// begin command buffer recording
	for (size_t i = 0; i < commandBuffers.size(); ++i)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;		// Optional
		beginInfo.pInheritanceInfo = nullptr;  // Optional (use when using secondary command buffers)

		// BEGIN 
		// RECORD COMMANDS
		// END

		result = vkBeginCommandBuffer(commandBuffers[i], &beginInfo);
		DJINN_VK_ASSERT(result);

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass.handle;
		renderPassInfo.framebuffer = p_swapChain->swapChainFramebuffers[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = p_swapChain->swapChainExtent;

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.NumElem());
		renderPassInfo.pClearValues = clearValues.Ptr();

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.pipeline);

		VkBuffer vertexBuffers[]{ _vertexBuffer.buffer };
		VkDeviceSize offsets[]{ 0 };
		vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffers[i], _indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);

		vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(vertexIndices.size()), 1, 0, 0, 0);
		vkCmdEndRenderPass(commandBuffers[i]);

		result = vkEndCommandBuffer(commandBuffers[i]);
		DJINN_VK_ASSERT(result);
	}
}

void Djinn::VulkanEngine::createSyncObjects()
{
	imagesInFlight.resize(p_swapChain->swapChainImages.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		auto result = (vkCreateSemaphore(p_context->gpuInfo.device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) == VK_SUCCESS &&
			vkCreateSemaphore(p_context->gpuInfo.device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) == VK_SUCCESS &&
			vkCreateFence(p_context->gpuInfo.device, &fenceInfo, nullptr, &inFlightFences[i]) == VK_SUCCESS);
		assert(result);
	}
}

void Djinn::VulkanEngine::drawFrame()
{
	// wait for fence from previous vkQueueSubmit call
	vkWaitForFences(p_context->gpuInfo.device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t swapChainImageIndex;
	// if we acquire the image IMAGE_AVAILABLE semaphore will be signaled
	auto result{ vkAcquireNextImageKHR(p_context->gpuInfo.device, p_swapChain->swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &swapChainImageIndex) };
	assert(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);

	// check if a previous frame is using this image
	if (imagesInFlight[swapChainImageIndex] != VK_NULL_HANDLE)
	{
		vkWaitForFences(p_context->gpuInfo.device, 1, &imagesInFlight[swapChainImageIndex], VK_TRUE, UINT64_MAX);
	}

	// mark image as "in-use"
	imagesInFlight[swapChainImageIndex] = inFlightFences[currentFrame];

	updateUniformBuffer(swapChainImageIndex);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	// if IMAGE_AVAILABLE - We can submit to the queue
	VkPipelineStageFlags waitStages{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAvailableSemaphores[currentFrame];
	submitInfo.pWaitDstStageMask = &waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[swapChainImageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentFrame];

	vkResetFences(p_context->gpuInfo.device, 1, &inFlightFences[currentFrame]);
	result = vkQueueSubmit(p_context->graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]);
	DJINN_VK_ASSERT(result);

	// if RENDER_FINISHED - we can present the image to the screen
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &p_swapChain->swapChain;
	presentInfo.pImageIndices = &swapChainImageIndex;
	presentInfo.pResults = nullptr;		// Optional

	result = vkQueuePresentKHR(p_context->presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || p_context->framebufferResized)
	{
		p_context->framebufferResized = false;
		recreateSwapChain();
	}
	else if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to present swapchain image!");
	}

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Djinn::VulkanEngine::initImGui()
{
	//1: create descriptor pool for IMGUI
		// the size of the pool is very oversize, but its copied from imgui demo itself.
	VkDescriptorPoolSize poolSizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo poolCreateInfo{};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolCreateInfo.maxSets = 1000;
	poolCreateInfo.poolSizeCount = std::size(poolSizes);
	poolCreateInfo.pPoolSizes = poolSizes;

	VkDescriptorPool imguiPool;
	DJINN_VK_ASSERT(vkCreateDescriptorPool(p_context->gpuInfo.device, &poolCreateInfo, nullptr, &imguiPool));

	// init imgui

}




