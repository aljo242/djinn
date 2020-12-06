#ifndef DEBUG_MESSENGER_H
#define DEBUG_MESSENGER_H

#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>

class DebugMessenger
{
public:
	uint32_t GetErrorCount() const;
	uint32_t GetWarningCount() const;
	uint32_t GetInfoCount() const;

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagBitsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		auto pThis = static_cast<DebugMessenger*>(pUserData);
		return pThis->Log(messageSeverity, messageType, pCallbackData);
	}
	
	VkDebugUtilsMessengerEXT handle{ VK_NULL_HANDLE };

protected:
	uint32_t errorCount{0};
	uint32_t warningCount{0};
	uint32_t infoCount{0};

	VkBool32 Log(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagBitsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData);
};

#endif
