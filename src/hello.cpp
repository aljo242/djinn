#include "hello.h"
#include "gfxDebug.h"

#include <chrono>

const std::vector<Vertex> vertices  
{
	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
	{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};

const std::vector<uint16_t> vertexIndices 
{
	0, 1, 2, 2, 3, 0
};




HelloTriangleApp::HelloTriangleApp()
{
	initWindow();
	initVulkan();
}

void HelloTriangleApp::run()
{
	mainLoop();
	cleanup();
}


void HelloTriangleApp::initWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Window", nullptr, nullptr);

	// use this to get access private members in the resize callback fxn
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebufferResizedCallback);
}


void HelloTriangleApp::initVulkan()
{
	createInstance();			//
	setupDebugMessenger();		//
	createSurface();			//
	pickPhysicalDevice();		//
	createLogicalDevice();		//
	createSwapChain();			//
	createImageViews();			//
	createRenderPass();			//
	createDescriptorSetLayout();//
	createGraphicsPipeline();	//
	createFramebuffers();		//
	createCommandPool();		//
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
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		drawFrame();
	}
}

void HelloTriangleApp::cleanup()
{
	// wait for the device to not be "mid-work" before we destroy objects
	vkDeviceWaitIdle(device);

	cleanupSwapChain();

	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

	vkDestroyBuffer(device, vertexBuffer, nullptr);
	vkFreeMemory(device, vertexBufferMemory, nullptr);
	vkDestroyBuffer(device, indexBuffer, nullptr);
	vkFreeMemory(device, indexBufferMemory, nullptr);

	// destroy sync objects
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(device, gfxCommandPool, nullptr);
	vkDestroyCommandPool(device, transferCommandPool, nullptr);
	vkDestroyDevice(device, nullptr);

	if constexpr (enableValidationlayers)
	{
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
	glfwDestroyWindow(window);
	glfwTerminate();
}


void HelloTriangleApp::createInstance()
{
	if (enableValidationlayers && !checkValidationLayerSupport())
	{
		throw std::runtime_error("Validation layers requested, but not supported!");
	}

	// basic app description
	VkApplicationInfo appInfo{};
	appInfo.sType				= VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName    = "Hello Triangle";
	appInfo.applicationVersion  = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName         = "Djinn";
	appInfo.engineVersion       = VK_MAKE_VERSION(1, 0, 0);

	// check for the correct API version
	// Djinn currently will only support Vulkan spec 1.1 and higher
	if (vkGetInstanceProcAddr(NULL, "vkEnumerateInstanceVersion") == NULL)
	{
		throw std::runtime_error("Device only supports Vulkan 1.0.  Djinn currently supports Vulkan 1.1 and 1.2");
	}

	uint32_t vulkanVersion{ 0 };
	vkEnumerateInstanceVersion(&vulkanVersion);
	if (vulkanVersion < VK_API_VERSION_1_1)
	{
		throw std::runtime_error("Device only supports Vulkan 1.0.  Djinn currently supports Vulkan 1.1 and 1.2");
	}
	appInfo.apiVersion = vulkanVersion;

#if defined(_DEBUG)
	glfwExtensionCheck();
	vulkanExtensionCheck();
#endif

	uint32_t glfwExtensionCount	  {0};
	const char** glfwExtensions{ glfwGetRequiredInstanceExtensions(&glfwExtensionCount) };

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo			= &appInfo;
	createInfo.enabledExtensionCount	= glfwExtensionCount;
	createInfo.ppEnabledExtensionNames	= glfwExtensions;

	// validation layers + debug info
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if constexpr (enableValidationlayers)
	{
		createInfo.enabledLayerCount	= static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames	= validationLayers.data();
		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext				= (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount	= 0;
		createInfo.pNext				= nullptr;
	}

	// extension info
	const auto extensions				{getRequiredExtensions()};
	createInfo.enabledExtensionCount	= static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames  = extensions.data();

	auto result {vkCreateInstance(&createInfo, nullptr, &instance)};
	assert(result == VK_SUCCESS);
}

bool HelloTriangleApp::checkValidationLayerSupport()
{
	uint32_t layerCount			{0};
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers)
	{
		bool layerFound		{false};
		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
		{
			return false;
		}
	}

	return true;
}

void HelloTriangleApp::glfwExtensionCheck()
{
	uint32_t glfwExtensionCount{ 0 };
	const char** glfwExtensions{ glfwGetRequiredInstanceExtensions(&glfwExtensionCount) };

	spdlog::debug("{} Available GLFW Extensions:", glfwExtensionCount);

	for (size_t i = 0; i < glfwExtensionCount; ++i)
	{
		spdlog::debug("\t {}", glfwExtensions[i]);
	}
}

void HelloTriangleApp::vulkanExtensionCheck()
{
	uint32_t extensionCount{ 0 };
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	spdlog::debug("{} Available Vulkan Extensions:", extensionCount);

	for (const auto& extension : extensions)
	{
		spdlog::debug("\t {}", extension.extensionName);
	}
}

void HelloTriangleApp::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = (PFN_vkDebugUtilsMessengerCallbackEXT)debugCallback;
	createInfo.pUserData = nullptr; // optional data		
}

void HelloTriangleApp::setupDebugMessenger()
{
	if(!enableValidationlayers) 
	{
		return;
	}

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	populateDebugMessengerCreateInfo(createInfo);

	auto result {CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger)};
	assert(result == VK_SUCCESS);
}

std::vector<const char*> HelloTriangleApp::getRequiredExtensions()
{
	uint32_t glfwExtensionCount		{0};
	const char** glfwExtensions		{glfwGetRequiredInstanceExtensions(&glfwExtensionCount)};

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount); // begin and end pointers

	if constexpr (enableValidationlayers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

// TODO expand this
uint32_t HelloTriangleApp::rateDeviceSuitability(VkPhysicalDevice physicalDev)
{
	VkPhysicalDeviceProperties2 deviceProperties;
	deviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;

	// TODO use pNext to query specific necessary features
	deviceProperties.pNext = nullptr;
	vkGetPhysicalDeviceProperties2(physicalDev, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(physicalDev, &deviceFeatures);

	uint32_t score					{0};

	//  huge score boost for discrete GPUs
	if (deviceProperties.properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_OTHER)
	{
		score += 1000 * (4 - deviceProperties.properties.deviceType);
	}

	// prefer device with 1.2 over 1.1
	if (deviceProperties.properties.apiVersion >= VK_API_VERSION_1_2)
	{
		score += 500;
	}

	// maximum possible size of textures affect graphics quality
	score += deviceProperties.properties.limits.maxImageDimension2D;

	// don't care if we can't use geometry shaders
	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPhysicalDeviceFeatures.html
	// if engine requires any of these features, may be required like shown 
	// below with geometry shader requirements
	if (!deviceFeatures.geometryShader)
	{
		return 0;
	}

	if (!checkDeviceExtensionSupport(physicalDev))
	{
		return 0;
	}

	// check if we support graphics queues and presentation
	QueueFamilyIndices indices					{findQueueFamilies(physicalDev, surface)};
	if (!indices.isComplete())
	{
		return 0;
	}

	// prefer to do graphics and presentation on the same device
	if (indices.graphicsFamily.value() == indices.presentFamily.value())
	{
		score += 1000;
	}
	
	SwapChainSupportDetails swapChainSupport	{querySwapChainSupport(physicalDev,	surface)};
	// if we support formats AND present modes, swapchain support is "adequate"
	bool swapChainAdequate{ !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty() };
	if (!swapChainAdequate)
	{
		return 0;
	}

	return score;
}

void HelloTriangleApp::createSurface()
{
	// exposing vulkan functions of glfw, so window creation becomes simplified
	auto result {(glfwCreateWindowSurface(instance, window, nullptr, &surface))};
	assert(result == VK_SUCCESS);
}

// build up a set up of required extensions
// check if required extensions is a subset of available extensions
// report if true or false
bool HelloTriangleApp::checkDeviceExtensionSupport(VkPhysicalDevice physicalDev)
{
	uint32_t extensionCount		{0};
	vkEnumerateDeviceExtensionProperties(physicalDev, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDev, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

void HelloTriangleApp::pickPhysicalDevice()
{
	uint32_t deviceCount		{0};
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	if (deviceCount == 0)
	{
		throw std::runtime_error("Failed to find GPUs with Vulkan support!");
	}
	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

	// <score, device number>
	std::multimap<uint32_t, VkPhysicalDevice> deviceCandidates;

	for (const auto& dev : physicalDevices)
	{
		uint32_t score			{rateDeviceSuitability(dev)};
		deviceCandidates.insert(std::make_pair(score, dev));
	}
		
	// choose the best candidate
	// rbegin will choose the highest score
	// reject if there are none greater than 0 as rateDeviceSuitability 
	// only returns 0 if the device is NOT suitable
	if (deviceCandidates.rbegin()->first > 0)
	{
		physicalDevice = deviceCandidates.rbegin()->second;
	}
	else
	{
		throw std::runtime_error("Failed to find a suitable GPU");
	}
}

VkPhysicalDeviceFeatures HelloTriangleApp::populateDeviceFeatures()
{
	VkPhysicalDeviceFeatures deviceFeatures{};

	// TODO 
	// add all needed features
	deviceFeatures.geometryShader			= VK_TRUE;
	deviceFeatures.wideLines				= VK_TRUE;

	// multi sampling features
	deviceFeatures.variableMultisampleRate	= VK_TRUE;
	deviceFeatures.alphaToOne				= VK_TRUE;
	deviceFeatures.sampleRateShading		= VK_TRUE;

	return deviceFeatures;
}

void HelloTriangleApp::createLogicalDevice()
{
	QueueFamilyIndices indices		{findQueueFamilies(physicalDevice, surface)};

	// if both queues have the same indices
	std::set<uint32_t> uniqueQueueFamilies {indices.graphicsFamily.value(), indices.presentFamily.value(), indices.transferFamily.value()};
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(uniqueQueueFamilies.size());

	constexpr float queuePriority{ 1.0f };
	size_t i{ 0 };
	for (const auto& queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType				= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex	= queueFamily;
		queueCreateInfo.queueCount			= 1;
		queueCreateInfo.pQueuePriorities	= &queuePriority;
		queueCreateInfos[i]					= queueCreateInfo;
		++i;
	}

	VkPhysicalDeviceFeatures deviceFeatures{ populateDeviceFeatures() };
	VkDeviceCreateInfo createInfo{};
	createInfo.sType						= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos			= queueCreateInfos.data();
	createInfo.queueCreateInfoCount			= static_cast<uint32_t>(queueCreateInfos.size());;
	createInfo.pEnabledFeatures				= &deviceFeatures;
	createInfo.enabledExtensionCount		= static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames		= deviceExtensions.data(); 


	// DEPRECATED
	// consider removing
	if constexpr (enableValidationlayers)
	{
		createInfo.enabledLayerCount		= static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames		= validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount		= 0;
	}

	auto result {(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device))};
	assert(result == VK_SUCCESS);
		
	// use created Logical Device to fill VkQueue objects with their proper interfaces
	// queues are automatically created with the Logical Device, which makes sense
	// since we device creation info includes queue creation info
	// all we need to do is GET them
	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	vkGetDeviceQueue(device, indices.transferFamily.value(), 0, &transferQueue);

}

void HelloTriangleApp::createSwapChain()
{
	SwapChainSupportDetails swapChainSupport	{querySwapChainSupport(physicalDevice, surface)};
	VkSurfaceFormatKHR surfaceFormat			{chooseSwapChainFormat(swapChainSupport.formats)};
	VkPresentModeKHR presentMode				{chooseSwapChainPresentMode(swapChainSupport.presentModes)};
	VkExtent2D extent							{chooseSwapChainExtent(swapChainSupport.capabilities, window)};

	uint32_t imageCount							{swapChainSupport.capabilities.minImageCount + 1};

	// if maxImageCount == 0, there is no maximum number of images
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType						= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface						= surface;
	createInfo.minImageCount				= imageCount;
	createInfo.imageFormat					= surfaceFormat.format;
	createInfo.imageColorSpace				= surfaceFormat.colorSpace;
	createInfo.imageExtent					= extent;
	createInfo.imageArrayLayers				= 1; // specify more if doing stereoscopic 3D
	createInfo.imageUsage					= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; 
	// use VK_IMAGE_USAGE_TRANSFER_DST_BIT if post-processing steps desired

	// TODO REVISIT imageSharingMode 
	QueueFamilyIndices indices						{findQueueFamilies(physicalDevice, surface)};
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

	auto result {(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain))};
	assert(result == VK_SUCCESS);

	createSwapChainImages();

	// save these objects for later use when re-creating swapchains
	swapChainImageFormat					= surfaceFormat.format;
	swapChainExtent							= extent;
}

void HelloTriangleApp::cleanupSwapChain()
{
	for (auto framebuffer : swapChainFramebuffers)
	{
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}

	vkFreeCommandBuffers(device, gfxCommandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
	vkDestroyPipeline(device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);

	for (auto imageView : swapChainImageViews)
	{
		vkDestroyImageView(device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(device, swapChain, nullptr);

	for (size_t i = 0; i < swapChainImages.size(); ++i)
	{
		vkDestroyBuffer(device, uniformBuffers[i], nullptr);
		vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
	}

	vkDestroyDescriptorPool(device, descriptorPool, nullptr);

}

void HelloTriangleApp::recreateSwapChain()
{
	int width {0};
	int height {0};
	glfwGetFramebufferSize(window, &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(device);

	cleanupSwapChain();

	createSwapChain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createFramebuffers();
	createUniformBuffers();
	createDescriptorPool();		//
	createDescriptorSets();		//
	createCommandBuffers();
}

void HelloTriangleApp::createSwapChainImages()
{
	uint32_t imageCount			{0};
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
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
		VkImageViewCreateInfo createInfo{};
		createInfo.sType			= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image			= swapChainImages[i];
		createInfo.viewType			= VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format			= swapChainImageFormat;
		// default swizzle mapping
		createInfo.components.r		= VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g		= VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b		= VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a		= VK_COMPONENT_SWIZZLE_IDENTITY;
		// subresourceRange describes the image purpose
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		auto result {(vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]))};
		assert(result == VK_SUCCESS);
	}
}

void HelloTriangleApp::createRenderPass()
{
	// create attachment
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format					= swapChainImageFormat;
	colorAttachment.samples					= VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp					= VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp					= VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp			= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp			= VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout			= VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout				= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// ref to attachment describes it in a "higher order" way
	// provides uint32_t index
	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment			= 0;
	colorAttachmentRef.layout				= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint				= VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount			= 1;
	subpass.pColorAttachments				= &colorAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType					= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount			= 1;
	renderPassInfo.pAttachments				= &colorAttachment;
	renderPassInfo.subpassCount				= 1;
	renderPassInfo.pSubpasses				= &subpass;
	renderPassInfo.dependencyCount			= 1;
	renderPassInfo.pDependencies			= &dependency;

	auto result {(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass))};
	assert(result == VK_SUCCESS);
}

void HelloTriangleApp::createGraphicsPipeline()
{
	ShaderLoader vertShader("shader/vert.spv", device);
	ShaderLoader fragShader("shader/frag.spv", device);

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
	viewport.width										= static_cast<float>(swapChainExtent.width);
	viewport.height										= static_cast<float>(swapChainExtent.height);
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
	multisamplingCreateInfo.sampleShadingEnable			= VK_FALSE;
	multisamplingCreateInfo.rasterizationSamples		= VK_SAMPLE_COUNT_1_BIT;
	multisamplingCreateInfo.minSampleShading			= 1.0f;		// Optional
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

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType							= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount					= 1;					
	pipelineLayoutInfo.pSetLayouts						= &descriptorSetLayout;				
	pipelineLayoutInfo.pushConstantRangeCount			= 0;					// Optional
	pipelineLayoutInfo.pPushConstantRanges				= nullptr;				// Optional

	auto result {(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout))};
	assert(result == VK_SUCCESS);

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
	pipelineCreateInfo.renderPass						= renderPass;
	pipelineCreateInfo.subpass							= 0;
	pipelineCreateInfo.basePipelineHandle				= VK_NULL_HANDLE;		// Optional
	pipelineCreateInfo.basePipelineIndex				= -1;					// Optional

	result = (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &graphicsPipeline));
	assert(result == VK_SUCCESS);
}

void HelloTriangleApp::createFramebuffers()
{
	swapChainFramebuffers.resize(swapChainImageViews.size());

	for (size_t i = 0; i < swapChainImages.size(); ++i)
	{
		// the attachment for this buffer is the image view we already have created
		VkImageView attachments[]	{swapChainImageViews[i]};

		VkFramebufferCreateInfo framebufferCreateInfo{};
		framebufferCreateInfo.sType						= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass				= renderPass;
		framebufferCreateInfo.attachmentCount			= 1;
		framebufferCreateInfo.pAttachments				= attachments;
		framebufferCreateInfo.width						= swapChainExtent.width;
		framebufferCreateInfo.height					= swapChainExtent.height;
		framebufferCreateInfo.layers					= 1;

		auto result {(vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &swapChainFramebuffers[i]))};
		assert(result == VK_SUCCESS);
	}
}

void HelloTriangleApp::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices				{findQueueFamilies(physicalDevice, surface)};

	VkCommandPoolCreateInfo poolCreateInfo{};
	poolCreateInfo.sType								= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolCreateInfo.queueFamilyIndex						= queueFamilyIndices.graphicsFamily.value();
	poolCreateInfo.flags								= 0;	// Optional

	auto result {(vkCreateCommandPool(device, &poolCreateInfo, nullptr, &gfxCommandPool))};
	assert(result == VK_SUCCESS);

	poolCreateInfo.queueFamilyIndex = queueFamilyIndices.transferFamily.value();

	// transfer commands are short-lived, so this hint could lead to allocation optimizations
	poolCreateInfo.flags								= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

	result = (vkCreateCommandPool(device, &poolCreateInfo, nullptr, &transferCommandPool));
	assert(result == VK_SUCCESS);
}

void HelloTriangleApp::createDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding				= 0;
	uboLayoutBinding.descriptorType			= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount		= 1;
	uboLayoutBinding.stageFlags				= VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers		= nullptr; // opt

	VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
	layoutCreateInfo.sType					= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.bindingCount			= 1;
	layoutCreateInfo.pBindings				= &uboLayoutBinding;

	auto result { (vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr, &descriptorSetLayout))};
	assert(result == VK_SUCCESS);
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
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
	vkUnmapMemory(device, stagingBufferMemory);

	createBuffer(bufferSize, vertexBufferFlags, vertexMemoryFlags, vertexBuffer, vertexBufferMemory, 0);

	copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);

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
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertexIndices.data(), static_cast<size_t>(bufferSize));
	vkUnmapMemory(device, stagingBufferMemory);

	createBuffer(bufferSize, indexBufferFlags, indexMemoryFlags, indexBuffer, indexBufferMemory, 0);

	copyBuffer(stagingBuffer, indexBuffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void HelloTriangleApp::createUniformBuffers()
{
	constexpr VkDeviceSize bufferSize					{ sizeof(UniformBufferObject) };
	const size_t swapchainSize							{swapChainImages.size()};

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
	VkDescriptorPoolSize poolSize{};
	poolSize.type										= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount							= static_cast<uint32_t>(swapChainImages.size());

	VkDescriptorPoolCreateInfo poolCreateInfo{};
	poolCreateInfo.sType								= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.poolSizeCount						= 1;
	poolCreateInfo.pPoolSizes							= &poolSize;
	poolCreateInfo.maxSets								= static_cast<uint32_t>(swapChainImages.size());

	auto result { (vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &descriptorPool))};
	assert(result == VK_SUCCESS);
}


void HelloTriangleApp::createDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size(), descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType										= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool							= descriptorPool;
	allocInfo.descriptorSetCount						= static_cast<uint32_t>(swapChainImages.size());
	allocInfo.pSetLayouts								= layouts.data();

	descriptorSets.resize(swapChainImages.size());
	auto result {(vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()))};
	assert(result == VK_SUCCESS);

	for (size_t i = 0; i < swapChainImages.size(); ++i)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer							= uniformBuffers[i];
		bufferInfo.offset							= 0;
		bufferInfo.range							= VK_WHOLE_SIZE; // overwriting whole buffer

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType						= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet						= descriptorSets[i];
		descriptorWrite.dstBinding					= 0;
		descriptorWrite.dstArrayElement				= 0;
		descriptorWrite.descriptorType				= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount				= 1;
		descriptorWrite.pBufferInfo					= &bufferInfo;
		descriptorWrite.pImageInfo					= nullptr; // opt
		descriptorWrite.pTexelBufferView			= nullptr; // opt

		vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
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
	ubo.projection = glm::perspective(glm::radians(45.0f), (static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height)), 0.1f, 10.0f);
	ubo.projection[1][1] *= -1.0f;

	void* data;
	vkMapMemory(device, uniformBuffersMemory[imageIndex], 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(device, uniformBuffersMemory[imageIndex]);
}


void HelloTriangleApp::createBuffer(const VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
	VkBuffer& buffer, VkDeviceMemory& bufferMemory, const VkDeviceSize offset)
{
	QueueFamilyIndices queueFamilyIndices{ findQueueFamilies(physicalDevice, surface) };
	std::array<uint32_t, 2> queueFamilies{ queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.transferFamily.value() };

	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType					= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size					= size;
	bufferCreateInfo.usage					= usage;
	// currently always want concurrent usage because we want graphics, transfer queues to have access
	bufferCreateInfo.sharingMode			= sharingMode;			
	bufferCreateInfo.queueFamilyIndexCount	= static_cast<uint32_t>(queueFamilies.size());
	bufferCreateInfo.pQueueFamilyIndices	= queueFamilies.data();

	auto result {vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer)};
	assert(result == VK_SUCCESS);

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType							= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize				= memRequirements.size;
	allocInfo.memoryTypeIndex				= findMemoryType(memRequirements.memoryTypeBits, properties);

	// TODO : make custom allocator that manages this memory and passes offsets
	result	= vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory);
	assert(result == VK_SUCCESS);

	vkBindBufferMemory(device, buffer, bufferMemory, offset);
}

void HelloTriangleApp::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, const VkDeviceSize size)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level					= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool			= transferCommandPool;
	allocInfo.commandBufferCount    = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags					= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion;
	copyRegion.srcOffset		= 0;
	copyRegion.dstOffset		= 0;
	copyRegion.size				= size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
	
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount	= 1;
	submitInfo.pCommandBuffers		= &commandBuffer;

	vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(transferQueue);

	vkFreeCommandBuffers(device, transferCommandPool, 1, &commandBuffer);
}


uint32_t HelloTriangleApp::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

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
	commandBuffers.resize(swapChainFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType										= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool								= gfxCommandPool;
	allocInfo.level										= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount						= static_cast<uint32_t>(commandBuffers.size());

	auto result {vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data())};
	assert(result == VK_SUCCESS);

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
		assert(result == VK_SUCCESS);

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType							= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass						= renderPass;
		renderPassInfo.framebuffer						= swapChainFramebuffers[i];
		renderPassInfo.renderArea.offset				= {0, 0};	
		renderPassInfo.renderArea.extent				= swapChainExtent;

		VkClearValue clearColor							= {0.0f, 0.0f, 0.0f, 1.0f};
		renderPassInfo.clearValueCount					= 1;
		renderPassInfo.pClearValues						= &clearColor;

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

		VkBuffer vertexBuffers[] {vertexBuffer};
		VkDeviceSize offsets[]	{0};
		vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16);

		vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);

		vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(vertexIndices.size()), 1, 0, 0, 0);
		vkCmdEndRenderPass(commandBuffers[i]);

		result = vkEndCommandBuffer(commandBuffers[i]);
		assert(result == VK_SUCCESS);
	}	
}

void HelloTriangleApp::createSyncObjects()
{
	imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType									= VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType										= VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags										= VK_FENCE_CREATE_SIGNALED_BIT;

	for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		auto result = (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) == VK_SUCCESS &&
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) == VK_SUCCESS &&
			vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) == VK_SUCCESS);
		assert(result);
	}
}

void HelloTriangleApp::drawFrame()
{
	// wait for fence from previous vkQueueSubmit call
	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	// if we acquire the image IMAGE_AVAILABLE semaphore will be signaled
	auto result {vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex)};

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapChain();
		return;
	}

	assert(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);


	// check if a previous frame is using this image
	if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
	{
		vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
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

	vkResetFences(device, 1, &inFlightFences[currentFrame]);
	result = vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]);
	assert(result == VK_SUCCESS);
 
	// if RENDER_FINISHED - we can present the image to the screen
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType									= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount						= 1;
	presentInfo.pWaitSemaphores							= &renderFinishedSemaphores[currentFrame];
	presentInfo.swapchainCount							= 1;
	presentInfo.pSwapchains								= &swapChain;
	presentInfo.pImageIndices							= &imageIndex;
	presentInfo.pResults								= nullptr;		// Optional

	result = vkQueuePresentKHR(presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result != VK_SUBOPTIMAL_KHR || framebufferResized)
	{
		framebufferResized = false;
		recreateSwapChain();
	}
	else if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to present swapchain image!");
	}

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;		
}





