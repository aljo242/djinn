#ifndef INSTANCE_H
#define INSTANCE_H

#include "../DebugMessenger.h"
#include "../ext_inc.h"
#include "defs.h"
#include "core.h"
#include <vector>
#include <vulkan/vulkan.h>




namespace Djinn
{
	struct RendererConfig
	{
		VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT; // default to 1 sample
	};

	class Instance
	{
	public:
		Instance()
			: windowWidth(INITIAL_WIN_WIDTH), windowHeight(INITIAL_WIN_HEIGHT)
		{}
		void Init();
		void queryWindowSize();
		void CleanUp();

	private:
		bool checkValidationLayerSupport();
		void glfwExtensionCheck();
		void vulkanExtensionCheck();
		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		std::vector<const char*> getRequiredExtensions();
		uint32_t rateDeviceSuitability(VkPhysicalDevice physicalDev);
		VkSampleCountFlagBits getMaxUsableSampleCount();
		bool checkDeviceExtensionSupport(VkPhysicalDevice physicalDev);
		VkPhysicalDeviceFeatures populateDeviceFeatures();

	public:
		RendererConfig renderConfig;

		bool framebufferResized{false};

		GLFWwindow* window{ nullptr };
		int windowWidth{ 0 };
		int windowHeight{ 0 };

		VkInstance instance{ VK_NULL_HANDLE };
		VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
		VkDevice device{ VK_NULL_HANDLE };
		VkSurfaceKHR surface{ VK_NULL_HANDLE };

	private:
		DebugMessenger<DebugLevel::warning> debugMessenger{};
	};


	static void framebufferResizedCallback(GLFWwindow* window, const int width, const int height)
	{
		DJINN_UNUSED(width);
		DJINN_UNUSED(height);
		auto app{ reinterpret_cast<Instance*>(glfwGetWindowUserPointer(window)) };
		app->framebufferResized = true;
	}
}

#endif