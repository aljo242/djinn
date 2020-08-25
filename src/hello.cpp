#include <stdexcept>
#include <cstdlib>
#include <vector>

// caused glfw to automatically include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <spdlog/spdlog.h>


constexpr uint32_t WIDTH{ 800 };
constexpr uint32_t HEIGHT{ 600 };

const std::vector<const char*> validationLayers {"VK_LAYER_KHRONOS_validation"};

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
		if (enableValidationlayers)
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

		if (enableValidationlayers)
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
			spdlog::debug("Validation Layer: {}", pCallbackData->pMessage);
		}
		return VK_FALSE;
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
		if (enableValidationlayers)
		{
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}

		vkDestroyInstance(instance, nullptr);

		glfwDestroyWindow(window);

		glfwTerminate();
	}

private:
	// window data
	GLFWwindow* window;

	VkInstance instance;

	VkDebugUtilsMessengerEXT debugMessenger;
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