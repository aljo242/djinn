#include "DebugMessenger.h"

uint32_t DebugMessenger::GetErrorCount() const
{
	return errorCount;
}

uint32_t DebugMessenger::GetWarningCount() const 
{
	return warningCount;
}

uint32_t DebugMessenger::GetInfoCount() const
{
	return infoCount;
}

VkBool32 DebugMessenger::Log(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagBitsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData)
{
	if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		++warningCount;
		spdlog::warn("Validation Layer {}, Message: {}\n", messageType, pCallbackData->pMessage);
	}
	else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
	{
		++infoCount;
		spdlog::info("Validation Layer {}, Message: {}\n", messageType, pCallbackData->pMessage);
	}
	else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		++errorCount;
		spdlog::error("Validation Layer {}, Message: {}\n", messageType, pCallbackData->pMessage);
	}
	return VK_FALSE;
}

