#ifndef DEBUG_MESSENGER_H
#define DEBUG_MESSENGER_H

#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>

enum class DebugLevel
{
	info, warning, error
};

template <DebugLevel d>
class DebugMessenger
{

public:
	VkDebugUtilsMessengerEXT handle{ VK_NULL_HANDLE };

#if defined(_DEBUG)
	uint32_t GetErrorCount() const
	{
		return errorCount;
	}
	uint32_t GetWarningCount() const
	{
		return warningCount;
	}
	uint32_t GetInfoCount() const
	{
		return infoCount;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagBitsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		auto pThis = static_cast<DebugMessenger*>(pUserData);
		return pThis->Log(messageSeverity, messageType, pCallbackData);
	}
	

protected:
	uint32_t errorCount{0};
	uint32_t warningCount{0};
	uint32_t infoCount{0};

	VkBool32 Log(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagBitsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData)
	{
		if constexpr (d == DebugLevel::info)
		{
			if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
			{
				++infoCount;
				spdlog::info("Validation Layer {}, Message: {}\n", messageType, pCallbackData->pMessage);
			}
			else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			{
				++warningCount;
				spdlog::warn("Validation Layer {}, Message: {}\n", messageType, pCallbackData->pMessage);
			}
			else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
			{
				++errorCount;
				spdlog::error("Validation Layer {}, Message: {}\n", messageType, pCallbackData->pMessage);
			}
		}
		else if constexpr(d == DebugLevel::warning)
		{
			if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			{
				++warningCount;
				spdlog::warn("Validation Layer {}, Message: {}\n", messageType, pCallbackData->pMessage);
			}
			else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
			{
				++errorCount;
				spdlog::error("Validation Layer {}, Message: {}\n", messageType, pCallbackData->pMessage);
			}
		}
		else if constexpr (d == DebugLevel::error)
		{
			if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
			{
				++errorCount;
				spdlog::error("Validation Layer {}, Message: {}\n", messageType, pCallbackData->pMessage);
			}
		}
		return VK_FALSE;
	}
#endif
};


#endif
