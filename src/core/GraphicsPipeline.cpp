#include "GraphicsPipeline.h"
#include "Context.h"
#include "SwapChain.h"
#include "Primitives.h"


// init all fixed stages and cache
Djinn::GraphicsPipelineBuilder::GraphicsPipelineBuilder()
{
	vertexInputInfo = initVertexInputStageCreateInfo();
	viewportStateInfo = initViewPortStateCreateInfo();
	colorBlendAttachment = initColorBlendAttachmentState();
	colorBlendingInfo = initColorBlendingInfo();
	depthStencilCreateInfo = initDepthStencilCreateInfo();
}

Djinn::GraphicsPipeline Djinn::GraphicsPipelineBuilder::BuildPipeline(Djinn::Context* p_context, Djinn::SwapChain* p_swapchain, const PipelineConfig& config)
{
	VkPipeline newPipeline{ VK_NULL_HANDLE };
	VkPipelineLayout newPipelineLayout{ VK_NULL_HANDLE };
	viewport = initViewPort(p_swapchain);
	scissor = initScissor(p_swapchain);

	//std::vector<ShaderLoader> shaderLoaders;
	//for (const auto& createInfo : config.shaderLoadersCreateInfo)
	//{
	//	shaderLoaders.push_back(ShaderLoader(createInfo, p_context->gpuInfo.device));
	//}

	// take a vector of shaderloaders and resize shaderstage info
	for (const auto& shaderLoader : config.shaderLoaders)
	{
		shaderStageInfo.push_back(initPipelineShaderStageCreateInfo(shaderLoader));
	}

	inputAssemblyInfo = initInputAssemblyCreateInfo(config.primitiveTopology);
	rasterizerInfo = initRasterizationStateCreateInfo(config.polygonMode);
	multisamplingInfo = initMultiSamplingStageCreateInfo(config.msaaSamples);

	pipelineLayoutCreateInfo = initPipelineLayoutCreateInfo(config.descriptorSetLayouts);
	auto result = vkCreatePipelineLayout(p_context->gpuInfo.device, &pipelineLayoutCreateInfo, nullptr, &newPipelineLayout);
	DJINN_VK_ASSERT(result);

	//TODO 
	VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStageInfo.size());
	pipelineCreateInfo.pStages = shaderStageInfo.data();
	pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyInfo;
	pipelineCreateInfo.pViewportState = &viewportStateInfo;
	pipelineCreateInfo.pRasterizationState = &rasterizerInfo;
	pipelineCreateInfo.pMultisampleState = &multisamplingInfo;
	pipelineCreateInfo.pDepthStencilState = nullptr;				// Optional
	pipelineCreateInfo.pColorBlendState = &colorBlendingInfo;
	pipelineCreateInfo.pDynamicState = nullptr;				// Optional
	pipelineCreateInfo.layout = newPipelineLayout;
	pipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
	pipelineCreateInfo.renderPass = config.renderPass;
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;		// Optional
	pipelineCreateInfo.basePipelineIndex = -1;					// Optional

	result = vkCreateGraphicsPipelines(p_context->gpuInfo.device, nullptr, 1, &pipelineCreateInfo, nullptr, &newPipeline);
	DJINN_VK_ASSERT(result);

	if (newPipeline == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Failed to create new Vulkan pipeline!");
	}
	
	return { newPipeline, newPipelineLayout };
}

VkPipelineShaderStageCreateInfo Djinn::GraphicsPipelineBuilder::initPipelineShaderStageCreateInfo(const ShaderLoader& shaderLoader)
{
	VkPipelineShaderStageCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	info.pNext = nullptr;
	info.stage = shaderLoader.stage;
	info.module = shaderLoader.shaderModule;
	info.pName = shaderLoader.pName;

	return info;
}


VkPipelineVertexInputStateCreateInfo Djinn::GraphicsPipelineBuilder::initVertexInputStageCreateInfo()
{
	const static auto bindingDescription{ Vertex::getBindingDescription() };
	const static auto attributeDescriptions{ Vertex::getAttributeDescriptions() };

	VkPipelineVertexInputStateCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	info.vertexBindingDescriptionCount = 1;
	info.pVertexBindingDescriptions = &bindingDescription;
	info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.NumElem());
	info.pVertexAttributeDescriptions = attributeDescriptions.Ptr();

	return info;
}

VkPipelineInputAssemblyStateCreateInfo Djinn::GraphicsPipelineBuilder::initInputAssemblyCreateInfo(const VkPrimitiveTopology topology)
{
	VkPipelineInputAssemblyStateCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	info.pNext = nullptr;

	info.flags = 0;
	info.topology = topology;
	// TODO configure
	info.primitiveRestartEnable = VK_FALSE;

	return info;
}

VkPipelineRasterizationStateCreateInfo Djinn::GraphicsPipelineBuilder::initRasterizationStateCreateInfo(const VkPolygonMode polygonMode)
{
	VkPipelineRasterizationStateCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	info.pNext = nullptr;

	info.depthClampEnable = VK_FALSE;
	// discard all primitives before the raster stage if enabled
	info.rasterizerDiscardEnable = VK_FALSE;

	info.polygonMode = polygonMode;
	info.lineWidth = 1.0f;
	// no backface cull
	info.cullMode = VK_CULL_MODE_BACK_BIT;
	info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	// no depth bias
	/*
		Bias = (float)DepthBias * r + SlopeScaledDepthBias * MaxDepthSlope;
		where r is the minimum representable value > 0 in the depth-buffer format converted to float32.

		For a 24-bit depth buffer, r = 1 / 2^24.
		Example: DepthBias = 100000 ==> Actual DepthBias = 100000/2^24 = .006

		For a 16-bit depth buffer, r = 1 / 2^16.
		Example: DepthBias = 100000 ==> Actual DepthBias = 100000/2^16 = 1.52
	*/
	info.depthBiasEnable = VK_FALSE;
	info.depthBiasConstantFactor = 0.0f;
	info.depthBiasClamp = 0.0f;
	info.depthBiasSlopeFactor = 0.0f;

	return info;
}

VkPipelineMultisampleStateCreateInfo Djinn::GraphicsPipelineBuilder::initMultiSamplingStageCreateInfo(const VkSampleCountFlagBits msaaSamples)
{
	VkPipelineMultisampleStateCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	info.pNext = nullptr;

	info.sampleShadingEnable = VK_FALSE;
	info.rasterizationSamples = msaaSamples;
	info.minSampleShading = 0.2f;				// Optional
	info.pSampleMask = nullptr;					// Optional
	info.alphaToCoverageEnable = VK_FALSE;		// Optional
	info.alphaToOneEnable = VK_FALSE;			// Optional

	return info;
}

VkPipelineColorBlendAttachmentState Djinn::GraphicsPipelineBuilder::initColorBlendAttachmentState()
{
	VkPipelineColorBlendAttachmentState info{};
	info.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
						  VK_COLOR_COMPONENT_G_BIT |
						  VK_COLOR_COMPONENT_B_BIT |
						  VK_COLOR_COMPONENT_A_BIT;
	
	// alpha blending
	info.blendEnable = VK_TRUE;
	info.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;  // Optional
	info.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // Optional
	info.colorBlendOp = VK_BLEND_OP_ADD;		// Optional
	info.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
	info.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	info.alphaBlendOp = VK_BLEND_OP_ADD;

	return info;
}

VkPipelineViewportStateCreateInfo Djinn::GraphicsPipelineBuilder::initViewPortStateCreateInfo()
{
	VkPipelineViewportStateCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	info.pNext = nullptr;

	info.viewportCount = 1;
	info.pViewports = &viewport;
	info.scissorCount = 1;
	info.pScissors = &scissor;

	return info;
}

VkPipelineColorBlendStateCreateInfo Djinn::GraphicsPipelineBuilder::initColorBlendingInfo()
{
	VkPipelineColorBlendStateCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	info.logicOpEnable = VK_FALSE;
	info.logicOp = VK_LOGIC_OP_COPY;		// Optional
	info.attachmentCount = 1;
	info.pAttachments = &colorBlendAttachment;
	info.blendConstants[0] = 0.0f;					// Optional
	info.blendConstants[1] = 0.0f;					// Optional
	info.blendConstants[2] = 0.0f;					// Optional
	info.blendConstants[3] = 0.0f;

	return info;
}

VkPipelineDepthStencilStateCreateInfo Djinn::GraphicsPipelineBuilder::initDepthStencilCreateInfo()
{
	VkPipelineDepthStencilStateCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	info.depthTestEnable = VK_TRUE;
	info.depthWriteEnable = VK_TRUE;
	info.depthCompareOp = VK_COMPARE_OP_LESS;
	info.depthBoundsTestEnable = VK_FALSE;
	info.minDepthBounds = 0.0f;
	info.maxDepthBounds = 1.0f;
	info.stencilTestEnable = VK_FALSE;

	return info;
}

VkRect2D Djinn::GraphicsPipelineBuilder::initScissor(Djinn::SwapChain* p_swapChain)
{
	VkRect2D t_scissor{};
	t_scissor.offset = { 0, 0 };
	t_scissor.extent = p_swapChain->swapChainExtent;
	return t_scissor;
}

VkViewport Djinn::GraphicsPipelineBuilder::initViewPort(Djinn::SwapChain* p_swapChain)
{
	VkViewport t_viewport{};
	t_viewport.x = 0.0f;
	t_viewport.y = 0.0f;
	t_viewport.width = static_cast<float>(p_swapChain->swapChainExtent.width);
	t_viewport.height = static_cast<float>(p_swapChain->swapChainExtent.height);
	t_viewport.minDepth = 0.0f;
	t_viewport.maxDepth = 1.0f;

	return t_viewport;
}

VkPipelineLayoutCreateInfo Djinn::GraphicsPipelineBuilder::initPipelineLayoutCreateInfo(const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
{
	VkPipelineLayoutCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	info.pNext = nullptr;

	// defaults
	info.flags = 0;
	info.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	info.pSetLayouts = descriptorSetLayouts.data();
	info.pushConstantRangeCount = 0;
	info.pPushConstantRanges = nullptr;

	return info;
}

VkPipelineDynamicStateCreateInfo Djinn::GraphicsPipelineBuilder::initDynamicStateCreateInfo()
{
	VkPipelineDynamicStateCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	info.dynamicStateCount = defaultDynamicStateSize;
	info.pDynamicStates = dynamicStates.Ptr();

	return info;
}





