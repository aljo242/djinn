#include <array>

#include "SwapChain.h"

#include "Context.h"
#include "Image.h"
#include "RenderPass.h"
//#include "../SwapChainSupportDetails.h"
#include "../QueueFamilies.h"


Djinn::SwapChain::SwapChain(Context* p_context)
{
	Init(p_context);
}


void Djinn::SwapChain::Init(Context* p_context)
{
	SwapChainSupportDetails swapChainSupport{ querySwapChainSupport(p_context) };
	VkSurfaceFormatKHR surfaceFormat{ chooseSwapChainFormat(swapChainSupport.formats) };
	VkPresentModeKHR presentMode{ chooseSwapChainPresentMode(swapChainSupport.presentModes) };
	VkExtent2D extent{ chooseSwapChainExtent(p_context, swapChainSupport.capabilities) };

	uint32_t imageCount{ swapChainSupport.capabilities.minImageCount + 1 };

	// if maxImageCount == 0, there is no maximum number of images
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = p_context->surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1; // specify more if doing stereoscopic 3D
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	// use VK_IMAGE_USAGE_TRANSFER_DST_BIT if post-processing steps desired

	// TODO REVISIT imageSharingMode 
	QueueFamilyIndices indices{ findQueueFamilies(p_context->gpuInfo.gpu, p_context->surface) };
	std::array<uint32_t, 2> queueFamilyIndices{ indices.graphicsFamily.value(), indices.transferFamily.value() };

	if (!indices.sameIndices())
	{
		sharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.imageSharingMode = sharingMode;
		createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
		createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
	}
	else
	{
		sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.imageSharingMode = sharingMode;
		createInfo.queueFamilyIndexCount = 1;
		createInfo.pQueueFamilyIndices = &queueFamilyIndices[0];
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform; // if choose current transform, do nothing
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;	// currently ignoring alpha channel - don't want to blend with other windows
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE; // ignored obscured for performance benefit
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	auto result{ (vkCreateSwapchainKHR(p_context->gpuInfo.device, &createInfo, nullptr, &swapChain)) };
	DJINN_VK_ASSERT(result);

	createSwapChainImages(p_context);

	// save these objects for later use when re-creating swapchains
	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;

	createSwapChainImageViews(p_context);
}

void Djinn::SwapChain::CleanUp(Context* p_context)
{

	for (auto framebuffer : swapChainFramebuffers)
	{
		vkDestroyFramebuffer(p_context->gpuInfo.device, framebuffer, nullptr);
	}

	for (auto imageView : swapChainImageViews)
	{
		vkDestroyImageView(p_context->gpuInfo.device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(p_context->gpuInfo.device, swapChain, nullptr);
}

void Djinn::SwapChain::createSwapChainImages(Context* p_context)
{
	uint32_t imageCount{ 0 };
	vkGetSwapchainImagesKHR(p_context->gpuInfo.device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(p_context->gpuInfo.device, swapChain, &imageCount, swapChainImages.data());

}

void Djinn::SwapChain::createSwapChainImageViews(Context* p_context)
{
	const auto swapChainImageSize{ swapChainImages.size() };
	swapChainImageViews.resize(swapChainImageSize);
	for (size_t i = 0; i < swapChainImageSize; ++i)
	{
		swapChainImageViews[i] = createImageView(p_context, swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}
}


void Djinn::SwapChain::createFramebuffers(Context* p_context, Image* colorImage, Image* depthImage, VkRenderPass& renderPass)
{
	swapChainFramebuffers.resize(swapChainImageViews.size());

	for (size_t i = 0; i < swapChainImages.size(); ++i)
	{
		// the attachment for this buffer is the image view we already have created
		std::array<VkImageView, 3> attachments = {
			colorImage->imageView,
			depthImage->imageView,
			swapChainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferCreateInfo{};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = renderPass;
		framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferCreateInfo.pAttachments = attachments.data();
		framebufferCreateInfo.width = swapChainExtent.width;
		framebufferCreateInfo.height = swapChainExtent.height;
		framebufferCreateInfo.layers = 1;

		auto result{ (vkCreateFramebuffer(p_context->gpuInfo.device, &framebufferCreateInfo, nullptr, &swapChainFramebuffers[i])) };
		DJINN_VK_ASSERT(result);
	}
}


void Djinn::SwapChain::createFramebuffers(Context* p_context, VkImageView& colorImageView, VkImageView& depthImageView, VkRenderPass& renderPass)
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
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = renderPass;
		framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferCreateInfo.pAttachments = attachments.data();
		framebufferCreateInfo.width = swapChainExtent.width;
		framebufferCreateInfo.height = swapChainExtent.height;
		framebufferCreateInfo.layers = 1;

		auto result{ (vkCreateFramebuffer(p_context->gpuInfo.device, &framebufferCreateInfo, nullptr, &swapChainFramebuffers[i])) };
		DJINN_VK_ASSERT(result);
	}
}

Djinn::SwapChainSupportDetails Djinn::querySwapChainSupport(Context* p_context)
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(p_context->gpuInfo.gpu, p_context->surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(p_context->gpuInfo.gpu, p_context->surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(p_context->gpuInfo.gpu, p_context->surface, &formatCount, details.formats.data());
	}

	uint32_t presentCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(p_context->gpuInfo.gpu, p_context->surface, &presentCount, nullptr);

	if (presentCount != 0)
	{
		details.presentModes.resize(presentCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(p_context->gpuInfo.gpu, p_context->surface, &presentCount, details.presentModes.data());
	}

	return details;
}

Djinn::SwapChainSupportDetails Djinn::querySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
	}

	uint32_t presentCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentCount, nullptr);

	if (presentCount != 0)
	{
		details.presentModes.resize(presentCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentCount, details.presentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR Djinn::chooseSwapChainFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
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
	// need to figure out what i think next best means first

	// default return 
	return availableFormats[0];
}

VkPresentModeKHR Djinn::chooseSwapChainPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes)
	{
		// allows us to create "triple buffering" schemes
		// may want to use immediate
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
	}

	// default return 
	// guaranteed to be present
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Djinn::chooseSwapChainExtent(Context* p_context, const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		return capabilities.currentExtent;
	}
	else
	{
		VkExtent2D actualExtent{ static_cast<uint32_t>(p_context->windowWidth), static_cast<uint32_t>(p_context->windowHeight) };

		// clamp within [capabilities.minextent, capabilities.maxextent]
		actualExtent.width = std::max(capabilities.minImageExtent.width,
			std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height,
			std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}