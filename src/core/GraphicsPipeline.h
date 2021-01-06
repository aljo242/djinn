#ifndef GRAPHICS_PIPELINE_INCLUDE_H
#define GRAPHICS_PIPELINE_INCLUDE_H

#include <vulkan/vulkan.h>
#include <vector>

#include "../DjinnLib/Array.h"
#include "../ShaderLoader.h"
 
namespace Djinn
{
	class Context;
	class SwapChain;

	struct PipelineConfig
	{
		VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
		VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
		VkPrimitiveTopology primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		VkRenderPass renderPass;
		std::vector<ShaderLoader> shaderLoaders;
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	};

	struct GraphicsPipeline
	{
		VkPipeline pipeline{ VK_NULL_HANDLE };
		VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
	};

	class GraphicsPipelineBuilder
	{
	public:
		std::vector<VkPipelineShaderStageCreateInfo> shaderStageInfo{};
		VkPipelineMultisampleStateCreateInfo multisamplingInfo{};
		VkPipelineRasterizationStateCreateInfo rasterizerInfo{};
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		VkPipelineColorBlendStateCreateInfo colorBlendingInfo{};
		VkPipelineViewportStateCreateInfo viewportStateInfo{};
		VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo{};
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};

		constexpr static size_t defaultDynamicStateSize{ 2 };
		Djinn::Array1D<VkDynamicState, defaultDynamicStateSize> dynamicStates{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH};

		VkViewport viewport;
		VkRect2D scissor;

		GraphicsPipelineBuilder();
		GraphicsPipeline BuildPipeline(Djinn::Context* p_context, Djinn::SwapChain* p_swapchain, const PipelineConfig& config);

	private:

		VkPipelineShaderStageCreateInfo initPipelineShaderStageCreateInfo(const ShaderLoader& shaderLoader);
		VkPipelineVertexInputStateCreateInfo initVertexInputStageCreateInfo();
		VkPipelineInputAssemblyStateCreateInfo initInputAssemblyCreateInfo(const VkPrimitiveTopology topology);
		VkPipelineRasterizationStateCreateInfo initRasterizationStateCreateInfo(const VkPolygonMode polygonMode);
		VkPipelineMultisampleStateCreateInfo initMultiSamplingStageCreateInfo(const VkSampleCountFlagBits msaaSamples);
		VkPipelineColorBlendAttachmentState initColorBlendAttachmentState();
		VkPipelineViewportStateCreateInfo initViewPortStateCreateInfo();
		VkPipelineColorBlendStateCreateInfo initColorBlendingInfo();
		VkPipelineDepthStencilStateCreateInfo initDepthStencilCreateInfo();
		VkRect2D initScissor(Djinn::SwapChain* p_swapChain);
		VkViewport initViewPort(Djinn::SwapChain* p_swapChain);
		VkPipelineLayoutCreateInfo initPipelineLayoutCreateInfo(const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);
		VkPipelineDynamicStateCreateInfo initDynamicStateCreateInfo();
	};
}

#endif // GRAPHICS_PIPELINE_INCLUDE_H