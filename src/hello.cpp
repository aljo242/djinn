#include "hello.h"
#include "core/Instance.h"
#include "core/SwapChain.h"
#include "core/defs.h"

#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include <tiny_obj_loader.h>


HelloTriangleApp::HelloTriangleApp()
{
	initVulkan();
}

HelloTriangleApp::~HelloTriangleApp()
{
	delete p_instance;
}

void HelloTriangleApp::run()
{
	mainLoop();
	cleanup();
}


void HelloTriangleApp::initVulkan()
{
	p_instance = new Djinn::Instance();
	p_instance->Init();
	auto indices{ findQueueFamilies(p_instance->physicalDevice, p_instance->surface) };

	vkGetDeviceQueue(p_instance->device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(p_instance->device, indices.presentFamily.value(), 0, &presentQueue);
	vkGetDeviceQueue(p_instance->device, indices.transferFamily.value(), 0, &transferQueue);

	msaaSamples = p_instance->renderConfig.msaaSamples;

	p_swapChain = new Djinn::SwapChain(p_instance);

	//createSwapChain();			//
	//createImageViews();			//
	createRenderPass();			//
	createDescriptorSetLayout();//
	createGraphicsPipeline();	//
	createColorResources();     //
	createDepthResources();		//
	//createFramebuffers();		//
	p_swapChain->createFramebuffers(p_instance, colorImageView, depthImageView, renderPass);
	createCommandPool();		//
	createTextureImage();		// 
	createTextureImageView();	//
	createTextureSampler();		//
	loadModel();				//
	createVertexBuffer();		//
	createIndexBuffer();		//
	createUniformBuffers();		//
	createDescriptorPool();		//
	createDescriptorSets();		//
	createCommandBuffers();		//
	createSyncObjects();		//
}

void HelloTriangleApp::mainLoop()
{
	while (!glfwWindowShouldClose(p_instance->window))
	{
		glfwPollEvents();
		drawFrame();
	}
}

void HelloTriangleApp::cleanup()
{
	// wait for the device to not be "mid-work" before we destroy objects
	vkDeviceWaitIdle(p_instance->device);

	//cleanupSwapChain();
	p_swapChain->CleanUp(p_instance);

	vkDestroySampler(p_instance->device, textureSampler, nullptr);
	vkDestroyImageView(p_instance->device, textureImageView, nullptr);

	vkDestroyDescriptorSetLayout(p_instance->device, descriptorSetLayout, nullptr);

	vkDestroyBuffer(p_instance->device, vertexBuffer, nullptr);
	vkFreeMemory(p_instance->device, vertexBufferMemory, nullptr);
	vkDestroyBuffer(p_instance->device, indexBuffer, nullptr);
	vkFreeMemory(p_instance->device, indexBufferMemory, nullptr);
	vkDestroyImage(p_instance->device, textureImage, nullptr);
	vkFreeMemory(p_instance->device, textureImageMemory, nullptr);

	// destroy sync objects
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroySemaphore(p_instance->device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(p_instance->device, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(p_instance->device, inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(p_instance->device, gfxCommandPool, nullptr);
	vkDestroyCommandPool(p_instance->device, transferCommandPool, nullptr);

	p_instance->CleanUp();
}

VkFormat HelloTriangleApp::findSupportedFormat(const std::vector<VkFormat>& candidates, const VkImageTiling tiling, const VkFormatFeatureFlags features)
{
	for (const auto& format : candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(p_instance->physicalDevice, format, &props);

		if ((tiling == VK_IMAGE_TILING_LINEAR) && (props.linearTilingFeatures & features) == features)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}

	throw std::runtime_error("Could not find supported format!");
}

VkFormat HelloTriangleApp::findDepthFormat()
{
	return findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

bool HelloTriangleApp::hasStencilComponent(const VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}


void HelloTriangleApp::createSwapChain()
{
	SwapChainSupportDetails swapChainSupport	{querySwapChainSupport(p_instance->physicalDevice, p_instance->surface)};
	VkSurfaceFormatKHR surfaceFormat			{chooseSwapChainFormat(swapChainSupport.formats)};
	VkPresentModeKHR presentMode				{chooseSwapChainPresentMode(swapChainSupport.presentModes)};
	VkExtent2D extent							{chooseSwapChainExtent(swapChainSupport.capabilities, p_instance->window)};

	uint32_t imageCount							{swapChainSupport.capabilities.minImageCount + 1};

	// if maxImageCount == 0, there is no maximum number of images
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType						= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface						= p_instance->surface;
	createInfo.minImageCount				= imageCount;
	createInfo.imageFormat					= surfaceFormat.format;
	createInfo.imageColorSpace				= surfaceFormat.colorSpace;
	createInfo.imageExtent					= extent;
	createInfo.imageArrayLayers				= 1; // specify more if doing stereoscopic 3D
	createInfo.imageUsage					= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; 
	// use VK_IMAGE_USAGE_TRANSFER_DST_BIT if post-processing steps desired

	// TODO REVISIT imageSharingMode 
	QueueFamilyIndices indices						{findQueueFamilies(p_instance->physicalDevice, p_instance->surface)};
	std::array<uint32_t, 2> queueFamilyIndices		{indices.graphicsFamily.value(), indices.transferFamily.value()};

	if (!indices.sameIndices())
	{
		sharingMode							= VK_SHARING_MODE_CONCURRENT;
		createInfo.imageSharingMode			= sharingMode;
		createInfo.queueFamilyIndexCount	= static_cast<uint32_t>(queueFamilyIndices.size());
		createInfo.pQueueFamilyIndices		= queueFamilyIndices.data();
	}
	else
	{
		sharingMode							= VK_SHARING_MODE_EXCLUSIVE;
		createInfo.imageSharingMode			= sharingMode;
		createInfo.queueFamilyIndexCount	= 1;
		createInfo.pQueueFamilyIndices		= &queueFamilyIndices[0];
	}

	createInfo.preTransform					= swapChainSupport.capabilities.currentTransform; // if choose current transform, do nothing
	createInfo.compositeAlpha				= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;	// currently ignoring alpha channel - don't want to blend with other windows
	createInfo.presentMode					= presentMode;
	createInfo.clipped						= VK_TRUE; // ignored obscured for performance benefit
	createInfo.oldSwapchain					= VK_NULL_HANDLE;

	auto result {(vkCreateSwapchainKHR(p_instance->device, &createInfo, nullptr, &swapChain))};
	DJINN_VK_ASSERT(result);

	createSwapChainImages();

	// save these objects for later use when re-creating swapchains
	swapChainImageFormat					= surfaceFormat.format;
	swapChainExtent							= extent;
}

void HelloTriangleApp::cleanupSwapChain()
{
	for (auto framebuffer : swapChainFramebuffers)
	{
		vkDestroyFramebuffer(p_instance->device, framebuffer, nullptr);
	}

	vkFreeCommandBuffers(p_instance->device, gfxCommandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
	vkDestroyPipeline(p_instance->device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(p_instance->device, pipelineLayout, nullptr);
	vkDestroyRenderPass(p_instance->device, renderPass, nullptr);

	for (auto imageView : swapChainImageViews)
	{
		vkDestroyImageView(p_instance->device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(p_instance->device, p_swapChain->swapChain, nullptr);

	for (size_t i = 0; i < swapChainImages.size(); ++i)
	{
		vkDestroyBuffer(p_instance->device, uniformBuffers[i], nullptr);
		vkFreeMemory(p_instance->device, uniformBuffersMemory[i], nullptr);
	}

	vkDestroyImageView(p_instance->device, colorImageView, nullptr);
	vkDestroyImage(p_instance->device, colorImage, nullptr);
	vkFreeMemory(p_instance->device, colorImageMemory, nullptr);

	vkDestroyImageView(p_instance->device, depthImageView, nullptr);
	vkDestroyImage(p_instance->device, depthImage, nullptr);
	vkFreeMemory(p_instance->device, depthImageMemory, nullptr);

	vkDestroyDescriptorPool(p_instance->device, descriptorPool, nullptr);

}

void HelloTriangleApp::recreateSwapChain()
{
	int width {0};
	int height {0};
	glfwGetFramebufferSize(p_instance->window, &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(p_instance->window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(p_instance->device);

	cleanupSwapChain();

	createSwapChain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createColorResources();
	createDepthResources();
	createFramebuffers();
	createUniformBuffers();
	createDescriptorPool();		//
	createDescriptorSets();		//
	createCommandBuffers();
}

void HelloTriangleApp::createSwapChainImages()
{
	uint32_t imageCount			{0};
	vkGetSwapchainImagesKHR(p_instance->device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(p_instance->device, swapChain, &imageCount, swapChainImages.data());
}

//	vkFrameBuffer wraps 
//	vkImageView wraps
//	vkImage
void HelloTriangleApp::createImageViews()
{
	const auto swapChainImageSize	{swapChainImages.size()};
	swapChainImageViews.resize(swapChainImageSize);
	for (size_t i = 0; i < swapChainImageSize; ++i)
	{
		swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}
}

void HelloTriangleApp::createRenderPass()
{
	// create attachment
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format					= p_swapChain->swapChainImageFormat;
	colorAttachment.samples					= msaaSamples;
	colorAttachment.loadOp					= VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp					= VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp			= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp			= VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout			= VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout				= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// ref to attachment describes it in a "higher order" way
	// provides uint32_t index
	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment			= 0;
	colorAttachmentRef.layout				= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;



	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = findDepthFormat();
	depthAttachment.samples = msaaSamples;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// color attachment 
	VkAttachmentDescription colorAttachmentResolve{};
	colorAttachmentResolve.format = p_swapChain->swapChainImageFormat;
	colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentResolveRef{};
	colorAttachmentResolveRef.attachment = 2;
	colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint				= VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount			= 1;
	subpass.pColorAttachments				= &colorAttachmentRef;
	subpass.pResolveAttachments				= &colorAttachmentResolveRef;
	subpass.pDepthStencilAttachment			= &depthAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 3> attachments {colorAttachment, depthAttachment, colorAttachmentResolve};

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType					= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount			= static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments				= attachments.data();
	renderPassInfo.subpassCount				= 1;
	renderPassInfo.pSubpasses				= &subpass;
	renderPassInfo.dependencyCount			= 1;
	renderPassInfo.pDependencies			= &dependency;

	auto result {(vkCreateRenderPass(p_instance->device, &renderPassInfo, nullptr, &renderPass))};
	DJINN_VK_ASSERT(result);
}

void HelloTriangleApp::createGraphicsPipeline()
{
	ShaderLoader vertShader("shader/vert.spv", p_instance->device);
	ShaderLoader fragShader("shader/frag.spv", p_instance->device);

	VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo{};
	vertShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageCreateInfo.module = vertShader.shaderModule;
	vertShaderStageCreateInfo.pName = vertShader.pName;

	VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo{};
	fragShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageCreateInfo.module = fragShader.shaderModule;
	fragShaderStageCreateInfo.pName = fragShader.pName;

	VkPipelineShaderStageCreateInfo shaderStages[]{ vertShaderStageCreateInfo, fragShaderStageCreateInfo };

	const auto bindingDescription		{Vertex::getBindingDescription() };
	const auto attributeDescriptions	{Vertex::getAttributeDescriptions()};

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{};
	vertexInputCreateInfo.sType							= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
	vertexInputCreateInfo.pVertexBindingDescriptions	= &bindingDescription;
	vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputCreateInfo.pVertexAttributeDescriptions	= attributeDescriptions.data();

	// triangle list with no index buffer
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
	inputAssemblyCreateInfo.sType						= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyCreateInfo.topology					= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyCreateInfo.primitiveRestartEnable		= VK_FALSE;

	VkViewport viewport{};
	viewport.x											= 0.0f;
	viewport.y											= 0.0f;
	viewport.width										= static_cast<float>(p_swapChain->swapChainExtent.width);
	viewport.height										= static_cast<float>(p_swapChain->swapChainExtent.height);
	viewport.minDepth									= 0.0f;
	viewport.maxDepth									= 1.0f;

	VkRect2D scissor{};
	scissor.offset										= {0, 0};
	scissor.extent										= swapChainExtent;

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
	viewportStateCreateInfo.sType						= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.viewportCount				= 1;
	viewportStateCreateInfo.pViewports					= &viewport;
	viewportStateCreateInfo.scissorCount				= 1;
	viewportStateCreateInfo.pScissors					= &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType									= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable							= VK_FALSE;
	rasterizer.rasterizerDiscardEnable					= VK_FALSE;
	rasterizer.polygonMode								= VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth								= 1.0f;
	rasterizer.cullMode									= VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace								= VK_FRONT_FACE_COUNTER_CLOCKWISE;

	// used during shadow mapping
	rasterizer.depthBiasEnable							= VK_FALSE;
	rasterizer.depthBiasConstantFactor					= 0.0f; // Optional
	rasterizer.depthBiasClamp							= 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor						= 0.0f; // Optional

	VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo{};
	multisamplingCreateInfo.sType						= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingCreateInfo.sampleShadingEnable			= VK_TRUE;
	multisamplingCreateInfo.rasterizationSamples		= msaaSamples;
	multisamplingCreateInfo.minSampleShading			= 0.2f;		// Optional
	multisamplingCreateInfo.pSampleMask					= nullptr;	// Optional
	multisamplingCreateInfo.alphaToCoverageEnable		= VK_FALSE; // Optional
	multisamplingCreateInfo.alphaToOneEnable			= VK_FALSE; // Optional

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask					= VK_COLOR_COMPONENT_R_BIT |
														  VK_COLOR_COMPONENT_G_BIT |
														  VK_COLOR_COMPONENT_B_BIT |															 
														  VK_COLOR_COMPONENT_A_BIT;

	// alpha blending
	colorBlendAttachment.blendEnable					= VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor			= VK_BLEND_FACTOR_SRC_ALPHA;  // Optional
	colorBlendAttachment.dstColorBlendFactor			= VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // Optional
	colorBlendAttachment.colorBlendOp					= VK_BLEND_OP_ADD;		// Optional
	colorBlendAttachment.srcAlphaBlendFactor			= VK_BLEND_FACTOR_ONE;  // Optional
	colorBlendAttachment.dstAlphaBlendFactor			= VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp					= VK_BLEND_OP_ADD;		// Optional

	VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo{};
	colorBlendingCreateInfo.sType						= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendingCreateInfo.logicOpEnable				= VK_FALSE;
	colorBlendingCreateInfo.logicOp						= VK_LOGIC_OP_COPY;		// Optional
	colorBlendingCreateInfo.attachmentCount				= 1;
	colorBlendingCreateInfo.pAttachments				= &colorBlendAttachment;
	colorBlendingCreateInfo.blendConstants[0]			= 0.0f;					// Optional
	colorBlendingCreateInfo.blendConstants[1]			= 0.0f;					// Optional
	colorBlendingCreateInfo.blendConstants[2]			= 0.0f;					// Optional
	colorBlendingCreateInfo.blendConstants[3]			= 0.0f;					// Optional

	constexpr size_t dynamicStatesSize					{2};
	VkDynamicState dynamicStates[dynamicStatesSize]		{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH};

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
	dynamicStateCreateInfo.sType						= VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.dynamicStateCount			= dynamicStatesSize;
	dynamicStateCreateInfo.pDynamicStates				= dynamicStates;

	// create depthStencil
	// convention used: LOWER DEPTH == CLOSER TO CAMERA
	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f;
	depthStencil.maxDepthBounds = 1.0f;
	depthStencil.stencilTestEnable = VK_FALSE;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType							= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount					= 1;					
	pipelineLayoutInfo.pSetLayouts						= &descriptorSetLayout;				
	pipelineLayoutInfo.pushConstantRangeCount			= 0;					// Optional
	pipelineLayoutInfo.pPushConstantRanges				= nullptr;				// Optional

	auto result {(vkCreatePipelineLayout(p_instance->device, &pipelineLayoutInfo, nullptr, &pipelineLayout))};
	DJINN_VK_ASSERT(result);

	VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.sType							= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount						= dynamicStatesSize;
	pipelineCreateInfo.pStages							= shaderStages;
	pipelineCreateInfo.pVertexInputState				= &vertexInputCreateInfo;
	pipelineCreateInfo.pInputAssemblyState				= &inputAssemblyCreateInfo;
	pipelineCreateInfo.pViewportState					= &viewportStateCreateInfo;
	pipelineCreateInfo.pRasterizationState				= &rasterizer;
	pipelineCreateInfo.pMultisampleState				= &multisamplingCreateInfo;
	pipelineCreateInfo.pDepthStencilState				= nullptr;				// Optional
	pipelineCreateInfo.pColorBlendState					= &colorBlendingCreateInfo; 
	pipelineCreateInfo.pDynamicState					= nullptr;				// Optional
	pipelineCreateInfo.layout							= pipelineLayout;
	pipelineCreateInfo.pDepthStencilState				= &depthStencil;
	pipelineCreateInfo.renderPass						= renderPass;
	pipelineCreateInfo.subpass							= 0;
	pipelineCreateInfo.basePipelineHandle				= VK_NULL_HANDLE;		// Optional
	pipelineCreateInfo.basePipelineIndex				= -1;					// Optional

	result = (vkCreateGraphicsPipelines(p_instance->device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &graphicsPipeline));
	DJINN_VK_ASSERT(result);
}

void HelloTriangleApp::createFramebuffers()
{
	swapChainFramebuffers.resize(swapChainImageViews.size());

	for (size_t i = 0; i < swapChainImages.size(); ++i)
	{
		// the attachment for this buffer is the image view we already have created
		std::array<VkImageView, 3> attachments = {
			colorImageView,
			depthImageView,
			swapChainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferCreateInfo{};
		framebufferCreateInfo.sType						= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass				= renderPass;
		framebufferCreateInfo.attachmentCount			= static_cast<uint32_t>(attachments.size());
		framebufferCreateInfo.pAttachments				= attachments.data();
		framebufferCreateInfo.width						= swapChainExtent.width;
		framebufferCreateInfo.height					= swapChainExtent.height;
		framebufferCreateInfo.layers					= 1;

		auto result {(vkCreateFramebuffer(p_instance->device, &framebufferCreateInfo, nullptr, &swapChainFramebuffers[i]))};
		DJINN_VK_ASSERT(result);
	}
}

void HelloTriangleApp::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices				{findQueueFamilies(p_instance->physicalDevice, p_instance->surface)};

	VkCommandPoolCreateInfo poolCreateInfo{};
	poolCreateInfo.sType								= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolCreateInfo.queueFamilyIndex						= queueFamilyIndices.graphicsFamily.value();
	poolCreateInfo.flags								= 0;	// Optional

	auto result {(vkCreateCommandPool(p_instance->device, &poolCreateInfo, nullptr, &gfxCommandPool))};
	DJINN_VK_ASSERT(result);

	poolCreateInfo.queueFamilyIndex = queueFamilyIndices.transferFamily.value();

	// transfer commands are short-lived, so this hint could lead to allocation optimizations
	poolCreateInfo.flags								= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

	result = (vkCreateCommandPool(p_instance->device, &poolCreateInfo, nullptr, &transferCommandPool));
	DJINN_VK_ASSERT(result);
}


void HelloTriangleApp::createDepthResources()
{
	VkFormat depthFormat {findDepthFormat()};

	createImage(p_swapChain->swapChainExtent.width, p_swapChain->swapChainExtent.height, 1, depthFormat, msaaSamples,
		 VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
	
	depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

void HelloTriangleApp::createTextureImage()
{
	int texWidth	{0};
	int texHeight	{0};
	int texChannels	{0};

	stbi_uc* pixels			{stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha)};
	assert(pixels != nullptr);
	const VkDeviceSize imageSize = texWidth * texHeight * 4;

	m_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, 0);

	void* data;
	vkMapMemory(p_instance->device, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(p_instance->device, stagingBufferMemory);

	stbi_image_free(pixels);

	createImage(texWidth, texHeight, m_mipLevels, VK_FORMAT_R8G8B8A8_SRGB, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);


	// transition image to transfer dst -> copy to image -> transfer to shader read only to sample from
	transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_mipLevels);
	copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
	//transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_mipLevels);

	vkDestroyBuffer(p_instance->device, stagingBuffer, nullptr);
	vkFreeMemory(p_instance->device, stagingBufferMemory, nullptr);

	generateMipMaps(textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, m_mipLevels);

}

VkImageView HelloTriangleApp::createImageView(const VkImage image, const VkFormat format, const VkImageAspectFlags aspectFlags, const uint32_t mipLevels)
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

	auto result{ vkCreateImageView(p_instance->device, &viewCreateInfo, nullptr, &imageView) };
	DJINN_VK_ASSERT(result);

	return imageView;
}

void HelloTriangleApp::createTextureImageView()
{
	textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, m_mipLevels);
}


void HelloTriangleApp::createTextureSampler()
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

	auto result {vkCreateSampler(p_instance->device, &samplerCreateInfo, nullptr, &textureSampler)};
	DJINN_VK_ASSERT(result);
}


void HelloTriangleApp::createColorResources()
{
	const auto colorFormat = p_swapChain->swapChainImageFormat;

	createImage(p_swapChain->swapChainExtent.width, p_swapChain->swapChainExtent.height, 1, colorFormat,
		msaaSamples, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		colorImage, colorImageMemory);
	colorImageView = createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

VkCommandBuffer HelloTriangleApp::beginSingleTimeCommands(VkCommandPool& commandPool	)
{
	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandPool = commandPool;
	allocateInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(p_instance->device, &allocateInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void HelloTriangleApp::endSingleTimeCommands(VkCommandPool& commandPool, VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	vkFreeCommandBuffers(p_instance->device, commandPool, 1, &commandBuffer);
}

// transition image layout by inserting image memory barrier to commandBuffer
void HelloTriangleApp::transitionImageLayout(const VkImage image, const VkFormat format, const VkImageLayout oldLayout, 
	const VkImageLayout newLayout, const uint32_t mipLevels)
{
	VkCommandBuffer commandBuffer {beginSingleTimeCommands(transferCommandPool)};

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

	endSingleTimeCommands(transferCommandPool, commandBuffer);
}


void HelloTriangleApp::copyBufferToImage(VkBuffer buffer, VkImage image, const uint32_t width, const uint32_t height)
{ 
	VkCommandBuffer commandBuffer	{beginSingleTimeCommands(transferCommandPool)};

	// copy the whole thing [0, 0, 0] -> [width, height, 1]
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = {0, 0, 0};
	region.imageExtent = {width, height, 1};

	// currently assumes layout has been transistioned to this state
	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	endSingleTimeCommands(transferCommandPool, commandBuffer);
}


void HelloTriangleApp::generateMipMaps(VkImage image, const VkFormat format, const uint32_t texWidth, const uint32_t texHeight, const uint32_t mipLevels)
{
	// check if image formats support linear blitting
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(p_instance->physicalDevice, format, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
	{
		throw std::runtime_error("texture image format does not support linear blitting!");
	}

	VkCommandPool commandPool = gfxCommandPool;
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
		blit.srcOffsets[0] = {0, 0, 0};
		blit.srcOffsets[1] = {mipWidth, mipHeight, 1};

		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = currentMipLevel;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;

		blit.dstOffsets[0] = {0, 0, 0};
		blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, 
			mipHeight > 1 ? mipHeight / 2 : 1, 
			1};

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

	endSingleTimeCommands(commandPool, commandBuffer);

}

void HelloTriangleApp::createImage(const uint32_t width, const uint32_t height, const uint32_t mipLevels, const VkFormat format,
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

	auto result{ vkCreateImage(p_instance->device, &imageCreateInfo, nullptr, &image) };
	DJINN_VK_ASSERT(result);

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(p_instance->device, image, &memRequirements);

	VkMemoryAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memRequirements.size;
	allocateInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	result = vkAllocateMemory(p_instance->device, &allocateInfo, nullptr, &imageMemory);
	DJINN_VK_ASSERT(result);

	result = vkBindImageMemory(p_instance->device, image, imageMemory, 0);
	DJINN_VK_ASSERT(result);
}


void HelloTriangleApp::loadModel()
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err;
	const char* path = MODEL_PATH.c_str();

	const auto ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, path);

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

			vertex.color = {1.0f, 1.0f, 1.0f};

			if (uniqueVertices.count(vertex) == 0)
			{
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}

			vertexIndices.push_back(uniqueVertices[vertex]);
		}
	}
}

void HelloTriangleApp::createDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType	= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.pImmutableSamplers = nullptr; // opt
	uboLayoutBinding.stageFlags	= VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding samplerLayourBinding{};
	samplerLayourBinding.binding = 1;
	samplerLayourBinding.descriptorCount = 1;
	samplerLayourBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayourBinding.pImmutableSamplers = nullptr;
	samplerLayourBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2> bindings {uboLayoutBinding, samplerLayourBinding};

	VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
	layoutCreateInfo.sType					= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.bindingCount			= static_cast<uint32_t>(bindings.size());
	layoutCreateInfo.pBindings				= bindings.data();

	auto result { (vkCreateDescriptorSetLayout(p_instance->device, &layoutCreateInfo, nullptr, &descriptorSetLayout))};
	DJINN_VK_ASSERT(result);
}


void HelloTriangleApp::createVertexBuffer()
{
	const VkDeviceSize bufferSize							{sizeof(vertices[0]) * vertices.size()};
	const VkBufferUsageFlags stagingBufferFlags				{VK_BUFFER_USAGE_TRANSFER_SRC_BIT};
	const VkMemoryPropertyFlags	stagingMemoryFlags			{ VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
															VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

	const VkBufferUsageFlags vertexBufferFlags				{ VK_BUFFER_USAGE_TRANSFER_DST_BIT | 
															VK_BUFFER_USAGE_VERTEX_BUFFER_BIT };
	const VkMemoryPropertyFlags	vertexMemoryFlags			{ VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(bufferSize, stagingBufferFlags, stagingMemoryFlags, stagingBuffer, stagingBufferMemory, 0);

	void* data;
	vkMapMemory(p_instance->device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
	vkUnmapMemory(p_instance->device, stagingBufferMemory);

	createBuffer(bufferSize, vertexBufferFlags, vertexMemoryFlags, vertexBuffer, vertexBufferMemory, 0);

	copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

	vkDestroyBuffer(p_instance->device, stagingBuffer, nullptr);
	vkFreeMemory(p_instance->device, stagingBufferMemory, nullptr);
}

void HelloTriangleApp::createIndexBuffer()
{
	const VkDeviceSize bufferSize							{ sizeof(vertexIndices[0]) * vertexIndices.size() };
	const VkBufferUsageFlags stagingBufferFlags				{ VK_BUFFER_USAGE_TRANSFER_SRC_BIT };
	const VkMemoryPropertyFlags	stagingMemoryFlags			{ VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
																VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

	const VkBufferUsageFlags indexBufferFlags				{ VK_BUFFER_USAGE_TRANSFER_DST_BIT |
																VK_BUFFER_USAGE_INDEX_BUFFER_BIT };
	const VkMemoryPropertyFlags	indexMemoryFlags			{ VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(bufferSize, stagingBufferFlags, stagingMemoryFlags, stagingBuffer, stagingBufferMemory, 0);

	void* data;
	vkMapMemory(p_instance->device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertexIndices.data(), static_cast<size_t>(bufferSize));
	vkUnmapMemory(p_instance->device, stagingBufferMemory);

	createBuffer(bufferSize, indexBufferFlags, indexMemoryFlags, indexBuffer, indexBufferMemory, 0);

	copyBuffer(stagingBuffer, indexBuffer, bufferSize);

	vkDestroyBuffer(p_instance->device, stagingBuffer, nullptr);
	vkFreeMemory(p_instance->device, stagingBufferMemory, nullptr);
}

void HelloTriangleApp::createUniformBuffers()
{
	constexpr VkDeviceSize bufferSize					{ sizeof(UniformBufferObject) };
	const size_t swapchainSize							{ p_swapChain->swapChainImages.size()};

	constexpr VkBufferUsageFlags uniformBufferFlags		{ VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT };
	constexpr VkMemoryPropertyFlags	uniformMemoryFlags	{ VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
														VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

	uniformBuffers.resize(swapchainSize);
	uniformBuffersMemory.resize(swapchainSize);

	for (size_t i = 0; i < swapchainSize; ++i)
	{
		createBuffer(bufferSize, uniformBufferFlags, uniformMemoryFlags, uniformBuffers[i], uniformBuffersMemory[i], 0);
	}
}

void HelloTriangleApp::createDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 2> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(p_swapChain->swapChainImages.size());
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(p_swapChain->swapChainImages.size());

	VkDescriptorPoolCreateInfo poolCreateInfo{};
	poolCreateInfo.sType								= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.poolSizeCount						= static_cast<uint32_t>(poolSizes.size());
	poolCreateInfo.pPoolSizes							= poolSizes.data();
	poolCreateInfo.maxSets								= static_cast<uint32_t>(p_swapChain->swapChainImages.size());

	auto result { (vkCreateDescriptorPool(p_instance->device, &poolCreateInfo, nullptr, &descriptorPool))};
	DJINN_VK_ASSERT(result);
}


void HelloTriangleApp::createDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(p_swapChain->swapChainImages.size(), descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType										= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool							= descriptorPool;
	allocInfo.descriptorSetCount						= static_cast<uint32_t>(p_swapChain->swapChainImages.size());
	allocInfo.pSetLayouts								= layouts.data();

	descriptorSets.resize(p_swapChain->swapChainImages.size());
	auto result {(vkAllocateDescriptorSets(p_instance->device, &allocInfo, descriptorSets.data()))};
	DJINN_VK_ASSERT(result);

	for (size_t i = 0; i < p_swapChain->swapChainImages.size(); ++i)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer							= uniformBuffers[i];
		bufferInfo.offset							= 0;
		bufferInfo.range							= sizeof(UniformBufferObject);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout =VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureImageView;
		imageInfo.sampler = textureSampler;

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

		descriptorWrites[0].sType						= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet						= descriptorSets[i];
		descriptorWrites[0].dstBinding					= 0;
		descriptorWrites[0].dstArrayElement				= 0;
		descriptorWrites[0].descriptorType				= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount				= 1;
		descriptorWrites[0].pBufferInfo					= &bufferInfo;
		descriptorWrites[0].pImageInfo					= nullptr; // opt
		descriptorWrites[0].pTexelBufferView			= nullptr; // opt

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pBufferInfo = nullptr; //opt
 		descriptorWrites[1].pImageInfo = &imageInfo; 
		descriptorWrites[1].pTexelBufferView = nullptr; // opt

		vkUpdateDescriptorSets(p_instance->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void HelloTriangleApp::updateUniformBuffer(const uint32_t imageIndex)
{
	static auto startTime								{std::chrono::high_resolution_clock::now()};
	const auto currentTime								{std::chrono::high_resolution_clock::now()};

	const float elapsedTime								{std::chrono::duration<float, std::chrono::seconds::period>
														(currentTime - startTime).count()};

	UniformBufferObject ubo{};
	ubo.model = glm::rotate(glm::mat4(1.0f), elapsedTime * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.projection = glm::perspective(glm::radians(45.0f), (static_cast<float>(p_swapChain->swapChainExtent.width) / static_cast<float>(p_swapChain->swapChainExtent.height)), 0.1f, 10.0f);
	ubo.projection[1][1] *= -1.0f;

	void* data;
	vkMapMemory(p_instance->device, uniformBuffersMemory[imageIndex], 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(p_instance->device, uniformBuffersMemory[imageIndex]);
}


void HelloTriangleApp::createBuffer(const VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
	VkBuffer& buffer, VkDeviceMemory& bufferMemory, const VkDeviceSize offset)
{
	QueueFamilyIndices queueFamilyIndices{ findQueueFamilies(p_instance->physicalDevice, p_instance->surface) };
	std::array<uint32_t, 2> queueFamilies{ queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.transferFamily.value() };

	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType					= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size					= size;
	bufferCreateInfo.usage					= usage;
	// currently always want concurrent usage because we want graphics, transfer queues to have access
	bufferCreateInfo.sharingMode			= p_swapChain->sharingMode;
	bufferCreateInfo.queueFamilyIndexCount	= static_cast<uint32_t>(queueFamilies.size());
	bufferCreateInfo.pQueueFamilyIndices	= queueFamilies.data();

	auto result {vkCreateBuffer(p_instance->device, &bufferCreateInfo, nullptr, &buffer)};
	DJINN_VK_ASSERT(result);

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(p_instance->device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType							= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize				= memRequirements.size;
	allocInfo.memoryTypeIndex				= findMemoryType(memRequirements.memoryTypeBits, properties);

	// TODO : make custom allocator that manages this memory and passes offsets
	result	= vkAllocateMemory(p_instance->device, &allocInfo, nullptr, &bufferMemory);
	DJINN_VK_ASSERT(result);

	vkBindBufferMemory(p_instance->device, buffer, bufferMemory, offset);
}

void HelloTriangleApp::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, const VkDeviceSize size)
{
	VkCommandBuffer commandBuffer {beginSingleTimeCommands(transferCommandPool)};

	VkBufferCopy copyRegion;
	copyRegion.srcOffset		= 0;
	copyRegion.dstOffset		= 0;
	copyRegion.size				= size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
	
	endSingleTimeCommands(transferCommandPool, commandBuffer);
}

uint32_t HelloTriangleApp::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(p_instance->physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
	{
		bool memDetect { (typeFilter & (1 << i)) && ((memProperties.memoryTypes[i].propertyFlags & properties) == properties) };
		if (memDetect)
		{
			return i;
		}
	}

	throw std::runtime_error("Failed to find suitable memory type!");

	return 0;
}

void HelloTriangleApp::createCommandBuffers()
{
	// create clear values
	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
	clearValues[1].depthStencil = {1.0f, 0};

	commandBuffers.resize(p_swapChain->swapChainFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType										= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool								= gfxCommandPool;
	allocInfo.level										= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount						= static_cast<uint32_t>(commandBuffers.size());

	auto result {vkAllocateCommandBuffers(p_instance->device, &allocInfo, commandBuffers.data())};
	DJINN_VK_ASSERT(result);

	// begin command buffer recording
	for (size_t i = 0; i < commandBuffers.size(); ++i)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType									= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags									= 0;		// Optional
		beginInfo.pInheritanceInfo						= nullptr;  // Optional (use when using secondary command buffers)


		// BEGIN 
		// RECORD COMMANDS
		// END

		result = vkBeginCommandBuffer(commandBuffers[i], &beginInfo);
		DJINN_VK_ASSERT(result);

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType							= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass						= renderPass;
		renderPassInfo.framebuffer						= p_swapChain->swapChainFramebuffers[i];
		renderPassInfo.renderArea.offset				= {0, 0};	
		renderPassInfo.renderArea.extent				= p_swapChain->swapChainExtent;

		renderPassInfo.clearValueCount					= static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues						= clearValues.data();

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

		VkBuffer vertexBuffers[] {vertexBuffer};
		VkDeviceSize offsets[]	{0};
		vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);

		vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(vertexIndices.size()), 1, 0, 0, 0);
		vkCmdEndRenderPass(commandBuffers[i]);

		result = vkEndCommandBuffer(commandBuffers[i]);
		DJINN_VK_ASSERT(result);
	}	
}

void HelloTriangleApp::createSyncObjects()
{
	imagesInFlight.resize(p_swapChain->swapChainImages.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType									= VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType										= VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags										= VK_FENCE_CREATE_SIGNALED_BIT;

	for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		auto result = (vkCreateSemaphore(p_instance->device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) == VK_SUCCESS &&
			vkCreateSemaphore(p_instance->device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) == VK_SUCCESS &&
			vkCreateFence(p_instance->device, &fenceInfo, nullptr, &inFlightFences[i]) == VK_SUCCESS);
		assert(result);
	}
}

void HelloTriangleApp::drawFrame()
{
	// wait for fence from previous vkQueueSubmit call
	vkWaitForFences(p_instance->device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	// if we acquire the image IMAGE_AVAILABLE semaphore will be signaled
	auto result {vkAcquireNextImageKHR(p_instance->device, p_swapChain->swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex)};

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapChain();
		return;
	}

	assert(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);


	// check if a previous frame is using this image
	if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
	{
		vkWaitForFences(p_instance->device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
	}

	// mark image as "in-use"
	imagesInFlight[imageIndex]							= inFlightFences[currentFrame];

	updateUniformBuffer(imageIndex);

	VkSubmitInfo submitInfo{};
	submitInfo.sType									= VK_STRUCTURE_TYPE_SUBMIT_INFO;

	// if IMAGE_AVAILABLE - We can submit to the queue
	VkPipelineStageFlags waitStages						{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount						= 1;
	submitInfo.pWaitSemaphores							= &imageAvailableSemaphores[currentFrame];
	submitInfo.pWaitDstStageMask						= &waitStages;
	submitInfo.commandBufferCount						= 1;
	submitInfo.pCommandBuffers							= &commandBuffers[imageIndex];
	submitInfo.signalSemaphoreCount						= 1;
	submitInfo.pSignalSemaphores						= &renderFinishedSemaphores[currentFrame];

	vkResetFences(p_instance->device, 1, &inFlightFences[currentFrame]);
	result = vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]);
	DJINN_VK_ASSERT(result);
 
	// if RENDER_FINISHED - we can present the image to the screen
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType									= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount						= 1;
	presentInfo.pWaitSemaphores							= &renderFinishedSemaphores[currentFrame];
	presentInfo.swapchainCount							= 1;
	presentInfo.pSwapchains								= &p_swapChain->swapChain;
	presentInfo.pImageIndices							= &imageIndex;
	presentInfo.pResults								= nullptr;		// Optional

	result = vkQueuePresentKHR(presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result != VK_SUBOPTIMAL_KHR || p_instance->framebufferResized)
	{
		p_instance->framebufferResized = false;
		recreateSwapChain();
	}
	else if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to present swapchain image!");
	}

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;		
}





