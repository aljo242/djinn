#include <stdexcept>
#include <cstdlib>
#include <cstdint> // UINT32_MAX
#include <vector>
#include <map>
#include <set>
#include <algorithm>

#include "ext_inc.h"

#include "QueueFamilies.h"
#include "SwapChainSupportDetails.h"
#include "ShaderLoader.h"



constexpr uint32_t WIDTH{ 800 };
constexpr uint32_t HEIGHT{ 600 };

const std::vector<const char*> validationLayers {"VK_LAYER_KHRONOS_validation"};
const std::vector<const char*> deviceExtensions {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#if defined(_DEBUG)
	constexpr bool enableValidationlayers	{true};
#else
	constexpr bool enableValidationlayers   {false};
#endif


VkResult CreateDebugUtilsMessengerEXT(
	VkInstance instance, 
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
	const VkAllocationCallbacks* pAllocator, 
	VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	const auto func		{(PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")};
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(
	VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger, 
	const VkAllocationCallbacks* pAllocator)
{
	const auto func		{(PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")};
	if (func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
}


VkSurfaceFormatKHR chooseSwapChainFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
	{
		// currently choosing 8-bit color with non-linear sRGB curve
		// https://stackoverflow.com/questions/12524623/what-are-the-practical-differences-when-working-with-colors-in-a-linear-vs-a-no
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	// TODO sort formats for the "next best"

	// default return 
	return availableFormats[0];
}

// TODO change to probably use another present mode
VkPresentModeKHR chooseSwapChainPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes)
	{
		// allows us to create "triple buffering" schemes
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
	}

	// default return
	return VK_PRESENT_MODE_FIFO_KHR;
}

// Swap Chain Extent is the resolution of the swap chain buffer image
VkExtent2D chooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		return capabilities.currentExtent;
	}
	else
	{
		VkExtent2D actualExtent {WIDTH, HEIGHT};
	
		actualExtent.width = std::max(capabilities.minImageExtent.width,
								std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height,
								std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}


class HelloTriangleApp
{
public:
	void run()
	{
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

private:
	void createInstance()
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
		appInfo.apiVersion			= VK_API_VERSION_1_2;

#if defined(_DEBUG)
		glfwExtensionCheck();
		vulkanExtensionCheck();
#endif

		uint32_t glfwExtensionCount	  {0};
		const char** glfwExtensions;

		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		//
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo			= &appInfo;
		createInfo.enabledExtensionCount	= glfwExtensionCount;
		createInfo.ppEnabledExtensionNames	= glfwExtensions;
		createInfo.enabledLayerCount		= 0;
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


		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create instance!");
		}
	}

	bool checkValidationLayerSupport()
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

	void glfwExtensionCheck()
	{
		uint32_t glfwExtensionCount{ 0 };
		const char** glfwExtensions{ glfwGetRequiredInstanceExtensions(&glfwExtensionCount) };

		spdlog::debug("{} Available GLFW Extensions:", glfwExtensionCount);

		for (size_t i = 0; i < glfwExtensionCount; ++i)
		{
			spdlog::debug("\t {}", glfwExtensions[i]);
		}
	}

	void vulkanExtensionCheck()
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

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = (PFN_vkDebugUtilsMessengerCallbackEXT)debugCallback;
		createInfo.pUserData = nullptr; // optional data		
	}

	void setupDebugMessenger()
	{
		if(!enableValidationlayers) 
		{
			return;
		}

		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		populateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to set up a debug messenger!");
		}
	}

	std::vector<const char*> getRequiredExtensions()
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

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
		VkDebugUtilsMessageTypeFlagBitsEXT messageType, 
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
		void* pUserData)
	{
		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			
			spdlog::debug("Validation Layer {} Message: {}", messageType, pCallbackData->pMessage);
		}
		return VK_FALSE;
	}

	// TODO expand this
	uint32_t rateDeviceSuitability(VkPhysicalDevice dev)
	{
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(dev, &deviceProperties);

		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(dev, &deviceFeatures);

		uint32_t score					{0};

		//  huge score boost for discrete GPUs
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			score += 1000;
		}

		// maximum possible size of textures affect graphics quality
		score += deviceProperties.limits.maxImageDimension2D;

		// don't care if we can't use geometry shaders
		if (!deviceFeatures.geometryShader)
		{
			return 0;
		}

		bool extensionsSupported		{checkDeviceExtensionSupport(dev)};

		if (!extensionsSupported)
		{
			return 0;
		}

		// check the command queue capabilities of the devices
		QueueFamilyIndices indices		{findQueueFamilies(dev, surface)};
		if (!indices.isComplete())
		{
			return 0;
		}

		bool swapChainAdequate{ false };
		// conditional will always be satisfied due to return statement "if (!extensionsSupported) {return 0;}"
		if (extensionsSupported)
		{
			SwapChainSupportDetails swapChainSupport	{querySwapChainSupport(dev,	surface)};
			// if we support formats AND present modes, swapchain support is "adequate"
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();

			if (!swapChainAdequate)
			{
				return 0;
			}
		}

		return score;
	}

	void createSurface()
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create window surface!");
		}
	}

	bool checkDeviceExtensionSupport(VkPhysicalDevice dev)
	{
		// TODO
		uint32_t extensionCount		{0};
		vkEnumerateDeviceExtensionProperties(dev, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(dev, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	void pickPhysicalDevice()
	{
		uint32_t deviceCount		{0};
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		if (deviceCount == 0)
		{
			throw std::runtime_error("Failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		std::multimap<uint32_t, VkPhysicalDevice> deviceCandidates;

		for (const auto& dev : devices)
		{
			uint32_t score			{rateDeviceSuitability(dev)};
			deviceCandidates.insert(std::make_pair(score, dev));
		}
		
		// choose the best candidate
		if (deviceCandidates.rbegin()->first > 0)
		{
			physicalDevice = deviceCandidates.rbegin()->second;
		}
		else
		{
			throw std::runtime_error("Failed to find a suitable GPU");
		}

	}

	void createLogicalDevice()
	{
		QueueFamilyIndices indices		{findQueueFamilies(physicalDevice, surface)};

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies {indices.graphicsFamily.value(), indices.presentFamily.value()};

		float queuePriority{ 1.0f };
		for (const auto& queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		VkDeviceCreateInfo createInfo{};
		createInfo.sType					= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos		= queueCreateInfos.data();
		createInfo.queueCreateInfoCount		= static_cast<uint32_t>(queueCreateInfos.size());;
		createInfo.pEnabledFeatures			= &deviceFeatures;
		createInfo.enabledExtensionCount	= static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames  = deviceExtensions.data();

		if constexpr (enableValidationlayers)
		{
			createInfo.enabledLayerCount	= static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames	= validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount	= 0;
		}

		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create logical device!");
		}
		
		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	}

	void createSwapChain()
	{
		SwapChainSupportDetails swapChainSupport	{querySwapChainSupport(physicalDevice, surface)};

		VkSurfaceFormatKHR surfaceFormat	{chooseSwapChainFormat(swapChainSupport.formats)};
		VkPresentModeKHR presentMode		{chooseSwapChainPresentMode(swapChainSupport.presentModes)};
		VkExtent2D extent					{chooseSwapChainExtent(swapChainSupport.capabilities)};

		uint32_t imageCount			{swapChainSupport.capabilities.minImageCount + 1};

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
		QueueFamilyIndices indices			{findQueueFamilies(physicalDevice, surface)};
		uint32_t queueFamilyIndices[]		{indices.graphicsFamily.value(), indices.presentFamily.value()};

		if (indices.graphicsFamily != indices.presentFamily)
		{
			createInfo.imageSharingMode			= VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount	= 2;
			createInfo.pQueueFamilyIndices		= queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode			= VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount	= 0;
			createInfo.pQueueFamilyIndices		= nullptr;
		}

		createInfo.preTransform					= swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha				= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;	// currently ignoring alpha channel
		createInfo.presentMode					= presentMode;
		createInfo.clipped						= VK_TRUE; // ignored obscured for performance benefit
		createInfo.oldSwapchain					= VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create swap chain!");
		}

		swapChainImageFormat					= surfaceFormat.format;
		swapChainExtent							= extent;
	}

	void createSwapChainImages()
	{
		uint32_t imageCount			{0};
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
	}

	void createImageViews()
	{
		swapChainImageViews.resize(swapChainImages.size());

		for (size_t i = 0; i < swapChainImages.size(); ++i)
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

			if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create an image view!");
			}
		}
	}

	void createGraphicsPipeline()
	{
		const auto vertShaderCode		{ readBinaryFile("shader/vert.spv") };
		const auto fragShaderCode		{ readBinaryFile("shader/frag.spv") };

	}

	void initWindow()
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		// TODO handle this and allow for resizable backbuffers
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Window", nullptr, nullptr);
	}

	void initVulkan()
	{
		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createSwapChainImages();
		createImageViews();
		createGraphicsPipeline();
	}

	void mainLoop()
	{
		while(!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
		}
	}

	void cleanup()
	{ 
		for (auto imageView : swapChainImageViews)
		{
			vkDestroyImageView(device, imageView, nullptr);
		}

		vkDestroySwapchainKHR(device, swapChain, nullptr);
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

private:
	// window data
	GLFWwindow* window								{nullptr};

	VkInstance instance								{ VK_NULL_HANDLE };
	VkPhysicalDevice physicalDevice					{ VK_NULL_HANDLE };		
	VkDevice device									{ VK_NULL_HANDLE };		
	VkQueue graphicsQueue							{ VK_NULL_HANDLE };
	VkQueue presentQueue							{ VK_NULL_HANDLE };
	VkSurfaceKHR surface							{ VK_NULL_HANDLE };

	VkDebugUtilsMessengerEXT debugMessenger			{ VK_NULL_HANDLE };
	
	VkSwapchainKHR swapChain						{ VK_NULL_HANDLE };
	VkFormat swapChainImageFormat;					
	VkExtent2D swapChainExtent;						
	std::vector<VkImage> swapChainImages;			
	std::vector<VkImageView> swapChainImageViews;	


};


int main()
{
#if defined(_DEBUG)
	spdlog::set_level(spdlog::level::debug);
#endif

	HelloTriangleApp app;

	try 
	{
		app.run();
	}
	catch(const std::exception& e)
	{
		spdlog::error("{}", e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}