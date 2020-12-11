#include <array>

#include "SwapChain.h"

#include "Instance.h"
#include "Image.h"
#include "RenderPass.h"
#include "../SwapChainSupportDetails.h"
#include "../QueueFamilies.h"


Djinn::SwapChain::SwapChain(Instance* p_instance)
{
	Init(p_instance);
}


void Djinn::SwapChain::Init(Instance* p_instance)
{
	SwapChainSupportDetails swapChainSupport{ querySwapChainSupport(p_instance->physicalDevice, p_instance->surface) };
	VkSurfaceFormatKHR surfaceFormat{ chooseSwapChainFormat(swapChainSupport.formats) };
	VkPresentModeKHR presentMode{ chooseSwapChainPresentMode(swapChainSupport.presentModes) };
	VkExtent2D extent{ chooseSwapChainExtent(swapChainSupport.capabilities, p_instance->window) };

	uint32_t imageCount{ swapChainSupport.capabilities.minImageCount + 1 };

	// if maxImageCount == 0, there is no maximum number of images
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = p_instance->surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1; // specify more if doing stereoscopic 3D
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	// use VK_IMAGE_USAGE_TRANSFER_DST_BIT if post-processing steps desired

	// TODO REVISIT imageSharingMode 
	QueueFamilyIndices indices{ findQueueFamilies(p_instance->physicalDevice, p_instance->surface) };
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

	auto result{ (vkCreateSwapchainKHR(p_instance->device, &createInfo, nullptr, &swapChain)) };
	DJINN_VK_ASSERT(result);

	createSwapChainImages(p_instance);

	// save these objects for later use when re-creating swapchains
	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;

	const auto swapChainImageSize{ swapChainImages.size() };
	swapChainImageViews.resize(swapChainImageSize);


	for (size_t i = 0; i < swapChainImageSize; ++i)
	{
		VkImageViewCreateInfo viewCreateInfo{};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.image = swapChainImages[i];
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.format = swapChainImageFormat;
		viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewCreateInfo.subresourceRange.baseMipLevel = 0;
		viewCreateInfo.subresourceRange.levelCount = 1;
		viewCreateInfo.subresourceRange.baseArrayLayer = 0;
		viewCreateInfo.subresourceRange.layerCount = 1;

		auto result{ vkCreateImageView(p_instance->device, &viewCreateInfo, nullptr, &swapChainImageViews[i]) };
		DJINN_VK_ASSERT(result);
	}
}

void Djinn::SwapChain::CleanUp(Instance* p_instance)
{

	for (auto framebuffer : swapChainFramebuffers)
	{
		vkDestroyFramebuffer(p_instance->device, framebuffer, nullptr);
	}

	for (auto imageView : swapChainImageViews)
	{
		vkDestroyImageView(p_instance->device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(p_instance->device, swapChain, nullptr);
}

void Djinn::SwapChain::createSwapChainImages(Instance* p_instance)
{
	uint32_t imageCount{ 0 };
	vkGetSwapchainImagesKHR(p_instance->device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(p_instance->device, swapChain, &imageCount, swapChainImages.data());
}

void Djinn::SwapChain::createFramebuffers(Instance* p_instance, Image* colorImage, Image* depthImage, RenderPass* renderPass)
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
		framebufferCreateInfo.renderPass = renderPass->renderPass;
		framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferCreateInfo.pAttachments = attachments.data();
		framebufferCreateInfo.width = swapChainExtent.width;
		framebufferCreateInfo.height = swapChainExtent.height;
		framebufferCreateInfo.layers = 1;

		auto result{ (vkCreateFramebuffer(p_instance->device, &framebufferCreateInfo, nullptr, &swapChainFramebuffers[i])) };
		DJINN_VK_ASSERT(result);
	}
}