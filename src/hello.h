#ifndef HELLO_H
#define HELLO_H

#include <stdexcept>
#include <cstdlib>
#include <cstdint> // UINT32_MAX
#include <vector>
#include <array>
#include <map>
#include <set>
#include <algorithm>

#include "ext_inc.h"
#include "QueueFamilies.h"
#include "SwapChainSupportDetails.h"
#include "ShaderLoader.h"
#include "core/core.h"
#include "core/Image.h"
#include <vulkan/vulkan.h>



constexpr uint32_t MAX_FRAMES_IN_FLIGHT{ 2 };


namespace Djinn 
{
	class Instance;
	class SwapChain;
}

struct Vertex
{
	glm::vec3 position;
	glm::vec3 color;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}

	bool operator==(const Vertex& other) const
	{
		return position == other.position && color == other.color && texCoord == other.texCoord;
	}
};

namespace std 
{
	template<> struct hash<Vertex>
	{
		size_t operator()(Vertex const& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.position) ^
					(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
					(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}


struct UniformBufferObject
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 projection;
};



class HelloTriangleApp
{
public:
	HelloTriangleApp();
	~HelloTriangleApp();
	void run();

private:
	void initVulkan();
	void mainLoop();
	void cleanup();

	// TODO expand this
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, const VkImageTiling tiling,
		const VkFormatFeatureFlags features);
	VkFormat findDepthFormat();
	bool hasStencilComponent(const VkFormat format);
	// Swap Chain Extent is the resolution of the swap chain buffer image
	void createSwapChain();
	void cleanupSwapChain();
	void recreateSwapChain();
	void createSwapChainImages();
	void createImageViews();
	void createRenderPass();
	void createGraphicsPipeline();
	void createFramebuffers();
	void createCommandPool();
	void createDepthResources();
	void createTextureImage();
	void createTextureImageView();
	void createTextureSampler();
	void createColorResources();
	VkImageView createImageView(const VkImage image, const VkFormat format, const VkImageAspectFlags aspectFlags, const uint32_t mipLevels);
	VkCommandBuffer beginSingleTimeCommands(VkCommandPool& commandPool);
	void endSingleTimeCommands(VkCommandPool& commandPool, VkCommandBuffer commandBuffer);
	void transitionImageLayout(VkImage image, const VkFormat format, const VkImageLayout oldLayout,
		const VkImageLayout newLayout, const uint32_t mipLevels);
	void copyBufferToImage(VkBuffer buffer, VkImage image, const uint32_t width, const uint32_t height);
	void generateMipMaps(VkImage image, const VkFormat format, const uint32_t texWidth, const uint32_t texHeight, const uint32_t mipLevels);
	void createImage(const uint32_t width, const uint32_t height, const uint32_t mipLevels, const VkFormat format,
		const VkSampleCountFlagBits numSamples, const VkImageTiling tiling, const VkImageUsageFlags flags, 
		const VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void loadModel();
	void createVertexBuffer();
	void createIndexBuffer();
	void createUniformBuffers();
	void createDescriptorPool();	
	void createDescriptorSets();
	void updateUniformBuffer(const uint32_t imageIndex);
	void createBuffer(const VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
		VkBuffer& buffer, VkDeviceMemory& bufferMemory, const VkDeviceSize offset);
	void createDescriptorSetLayout();
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, const VkDeviceSize size);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void createCommandBuffers();
	void createSyncObjects();
	void drawFrame();

	static void framebufferResizedCallback(GLFWwindow* window, const int width, const int height)
	{
		DJINN_UNUSED(width);
		DJINN_UNUSED(height);
		auto app{ reinterpret_cast<HelloTriangleApp*>(glfwGetWindowUserPointer(window)) };
		app->framebufferResized = true;
	}

private:

	Djinn::Instance* p_instance{ nullptr };
	Djinn::SwapChain* p_swapChain{ nullptr };

	VkQueue graphicsQueue							{ VK_NULL_HANDLE };
	VkQueue presentQueue							{ VK_NULL_HANDLE };
	VkQueue transferQueue							{ VK_NULL_HANDLE }; 

	VkDescriptorSetLayout descriptorSetLayout		{ VK_NULL_HANDLE };
	VkPipelineLayout pipelineLayout					{ VK_NULL_HANDLE };
	VkRenderPass renderPass							{ VK_NULL_HANDLE };
	VkPipeline graphicsPipeline						{ VK_NULL_HANDLE };

	VkCommandPool gfxCommandPool					{ VK_NULL_HANDLE };
	VkCommandPool transferCommandPool				{ VK_NULL_HANDLE };
	std::vector<VkCommandBuffer> commandBuffers;

	// synchronization
	std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> imageAvailableSemaphores;
	std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> renderFinishedSemaphores;
	std::array<VkFence, MAX_FRAMES_IN_FLIGHT> inFlightFences;
	std::vector<VkFence> imagesInFlight;
	size_t currentFrame{ 0 };

	bool framebufferResized{ false };

	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;

	// TODO 
	// combine vertex and index buffer int o a single array
	VkSharingMode sharingMode;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;

	VkImage textureImage;
	uint32_t m_mipLevels;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;

	// MSAA images
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT; // default to 1 sample
	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;
	Djinn::Image _colorImage;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;
	Djinn::Image _depthImage;


	// model info
	std::vector<Vertex> vertices;
	std::vector<uint32_t> vertexIndices;
};



#endif 