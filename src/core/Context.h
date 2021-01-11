#ifndef CONTEXT_INCLUDE_H
#define CONTEXT_INCLUDE_H

#include "../DebugMessenger.h"
#include "../ext_inc.h"
#include "../QueueFamilies.h"
#include "defs.h"
#include "core.h"
#include "IO.h"
#include <vector>
#include <vulkan/vulkan.h>



namespace Djinn
{
	struct RendererConfig
	{
		VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT; // default to 1 sample
	};

	struct GPU_Info
	{
		VkPhysicalDevice gpu;
		VkDevice device;
		VkPhysicalDeviceMemoryProperties memProperties;
		VkPhysicalDeviceProperties gpuProperties;
	};

	class Context
	{
	public:
		Context()
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
		void createCommandPools();

	public:
		RendererConfig renderConfig{};

		bool framebufferResized{false};

		GLFWwindow* window{ nullptr };
		int windowWidth{ 0 };
		int windowHeight{ 0 };

		VkInstance instance{ VK_NULL_HANDLE };
		VkSurfaceKHR surface{ VK_NULL_HANDLE };

		GPU_Info gpuInfo{};
		QueueFamilyIndices queueFamilyIndices;

		VkQueue graphicsQueue{ VK_NULL_HANDLE };
		VkQueue presentQueue{ VK_NULL_HANDLE };
		VkQueue transferQueue{ VK_NULL_HANDLE };

		VkCommandPool transferCommandPool{ VK_NULL_HANDLE };
		VkCommandPool graphicsCommandPool{ VK_NULL_HANDLE };

		Djinn::KeyboardState keyboardState;
		Djinn::MouseState mouseState;
		Djinn::GamepadState gamepadState;

	private:
		DebugMessenger<DebugLevel::warning> debugMessenger{};
	};


	static void framebufferResizedCallback(GLFWwindow* window, const int width, const int height)
	{
		DJINN_UNUSED(width);
		DJINN_UNUSED(height);
		auto app{ reinterpret_cast<Context*>(glfwGetWindowUserPointer(window)) };
		app->framebufferResized = true;
	}

	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		DJINN_UNUSED(scancode);
		DJINN_UNUSED(mods);
		auto app{ reinterpret_cast<Context*>(glfwGetWindowUserPointer(window)) };
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			app->keyboardState.a = DJINN_KEY_DOWN;

		}
		else if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			app->keyboardState.a = DJINN_KEY_UP;
		}

		if (key == GLFW_KEY_B && action == GLFW_PRESS)
		{
			app->keyboardState.b = DJINN_KEY_DOWN;
		}
		else if (key == GLFW_KEY_B && action == GLFW_RELEASE)
		{
			app->keyboardState.b = DJINN_KEY_UP;
		}

		if (key == GLFW_KEY_C && action == GLFW_PRESS)
		{
			app->keyboardState.c = DJINN_KEY_DOWN;
		}
		else if (key == GLFW_KEY_C && action == GLFW_RELEASE)
		{
			app->keyboardState.c = DJINN_KEY_UP;
		}

		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			app->keyboardState.d = DJINN_KEY_DOWN;
		}
		else if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			app->keyboardState.d = DJINN_KEY_UP;
		}

		if (key == GLFW_KEY_E && action == GLFW_PRESS)
		{
			app->keyboardState.e = DJINN_KEY_DOWN;
		}
		else if (key == GLFW_KEY_E && action == GLFW_RELEASE)
		{
			app->keyboardState.e = DJINN_KEY_UP;
		}

		if (key == GLFW_KEY_F && action == GLFW_PRESS)
		{
			app->keyboardState.f = DJINN_KEY_DOWN;
		}
		else if (key == GLFW_KEY_F && action == GLFW_RELEASE)
		{
			app->keyboardState.f = DJINN_KEY_UP;
		}

		if (key == GLFW_KEY_G && action == GLFW_PRESS)
		{
			app->keyboardState.g = DJINN_KEY_DOWN;
		}
		else if (key == GLFW_KEY_G && action == GLFW_RELEASE)
		{
			app->keyboardState.g = DJINN_KEY_UP;
		}

		if (key == GLFW_KEY_H && action == GLFW_PRESS)
		{
			app->keyboardState.h = DJINN_KEY_DOWN;
		}
		else if (key == GLFW_KEY_H && action == GLFW_RELEASE)
		{
			app->keyboardState.h = DJINN_KEY_UP;
		}

		if (key == GLFW_KEY_I && action == GLFW_PRESS)
		{
			app->keyboardState.i = DJINN_KEY_DOWN;
		}
		else if (key == GLFW_KEY_I && action == GLFW_RELEASE)
		{
			app->keyboardState.i = DJINN_KEY_UP;
		}

		if (key == GLFW_KEY_J && action == GLFW_PRESS)
		{
			app->keyboardState.j = DJINN_KEY_DOWN;
		}
		else if (key == GLFW_KEY_J && action == GLFW_RELEASE)
		{
			app->keyboardState.j = DJINN_KEY_UP;
		}

		if (key == GLFW_KEY_K && action == GLFW_PRESS)
		{
			app->keyboardState.k = DJINN_KEY_DOWN;
		}
		else if (key == GLFW_KEY_K && action == GLFW_RELEASE)
		{
			app->keyboardState.k = DJINN_KEY_UP;
		}

		if (key == GLFW_KEY_L && action == GLFW_PRESS)
		{
			app->keyboardState.l = DJINN_KEY_DOWN;
		}
		else if (key == GLFW_KEY_L && action == GLFW_RELEASE)
		{
			app->keyboardState.l = DJINN_KEY_UP;
		}

		if (key == GLFW_KEY_M && action == GLFW_PRESS)
		{
			app->keyboardState.m = DJINN_KEY_DOWN;
		}
		else if (key == GLFW_KEY_M && action == GLFW_RELEASE)
		{
			app->keyboardState.m = DJINN_KEY_UP;
		}

		if (key == GLFW_KEY_N && action == GLFW_PRESS)
		{
			app->keyboardState.n = DJINN_KEY_DOWN;
		}
		else if (key == GLFW_KEY_N && action == GLFW_RELEASE)
		{
			app->keyboardState.n = DJINN_KEY_UP;
		}

		if (key == GLFW_KEY_O && action == GLFW_PRESS)
		{
			app->keyboardState.o = DJINN_KEY_DOWN;
		}
		else if (key == GLFW_KEY_O && action == GLFW_RELEASE)
		{
			app->keyboardState.o = DJINN_KEY_UP;
		}

		if (key == GLFW_KEY_P && action == GLFW_PRESS)
		{
			app->keyboardState.p = DJINN_KEY_DOWN;
		}
		else if (key == GLFW_KEY_P && action == GLFW_RELEASE)
		{
			app->keyboardState.p = DJINN_KEY_UP;
		}

		if (key == GLFW_KEY_Q && action == GLFW_PRESS)
		{
			app->keyboardState.q = DJINN_KEY_DOWN;
		}
		else if (key == GLFW_KEY_Q && action == GLFW_RELEASE)
		{
			app->keyboardState.q = DJINN_KEY_UP;
		}

		if (key == GLFW_KEY_R && action == GLFW_PRESS)
		{
			app->keyboardState.r = DJINN_KEY_DOWN;
		}
		else if (key == GLFW_KEY_R && action == GLFW_RELEASE)
		{
			app->keyboardState.r = DJINN_KEY_UP;
		}

		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			app->keyboardState.s = DJINN_KEY_DOWN;
		}
		else if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			app->keyboardState.s = DJINN_KEY_UP;
		}

		if (key == GLFW_KEY_T && action == GLFW_PRESS)
		{
			app->keyboardState.t = DJINN_KEY_DOWN;
		}
		else if (key == GLFW_KEY_T && action == GLFW_RELEASE)
		{
			app->keyboardState.t = DJINN_KEY_UP;
		}

		if (key == GLFW_KEY_U && action == GLFW_PRESS)
		{
			app->keyboardState.u = DJINN_KEY_DOWN;
		}
		else if (key == GLFW_KEY_U && action == GLFW_RELEASE)
		{
			app->keyboardState.u = DJINN_KEY_UP;
		}

		if (key == GLFW_KEY_V && action == GLFW_PRESS)
		{
			app->keyboardState.v = DJINN_KEY_DOWN;
		}
		else if (key == GLFW_KEY_V && action == GLFW_RELEASE)
		{
			app->keyboardState.v = DJINN_KEY_UP;
		}

		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			app->keyboardState.w = DJINN_KEY_DOWN;
		}
		else if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			app->keyboardState.w = DJINN_KEY_UP;
		}

		if (key == GLFW_KEY_X && action == GLFW_PRESS)
		{
			app->keyboardState.x = DJINN_KEY_DOWN;
		}
		else if (key == GLFW_KEY_X && action == GLFW_RELEASE)
		{
			app->keyboardState.x = DJINN_KEY_UP;
		}

		if (key == GLFW_KEY_Y && action == GLFW_PRESS)
		{
			app->keyboardState.y = DJINN_KEY_DOWN;
		}
		else if (key == GLFW_KEY_Y && action == GLFW_RELEASE)
		{
			app->keyboardState.y = DJINN_KEY_UP;
		}

		if (key == GLFW_KEY_Z && action == GLFW_PRESS)
		{
			app->keyboardState.z = DJINN_KEY_DOWN;
		}
		else if (key == GLFW_KEY_Z && action == GLFW_RELEASE)
		{
			app->keyboardState.z = DJINN_KEY_UP;
		}
	}

	static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
	{
		auto app{ reinterpret_cast<Context*>(glfwGetWindowUserPointer(window)) };
		//spdlog::error("({}, {})", xpos, ypos);
		app->mouseState.x_pos = xpos;
		app->mouseState.y_pos = ypos;
	}
}

#endif // CONTEXT_INCLUDE_H