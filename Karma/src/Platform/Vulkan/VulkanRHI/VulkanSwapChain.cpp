#include "VulkanSwapChain.h"
#include "VulkanRHI/VulkanDynamicRHI.h"

#include "GLFW/glfw3.h"

namespace Karma
{
	FVulkanSwapChain* FVulkanSwapChain::Create(FVulkanDevice* InDevice)
	{
		FVulkanSwapChain* swapChain = new FVulkanSwapChain(InDevice);

		return swapChain;
	}

	FVulkanSwapChain::FVulkanSwapChain(FVulkanDevice* InDevice) : m_Instance(InDevice->GetVulkanDynamicRHI()->GetInstance()),
		m_Device(InDevice)
	{
		m_WindowHandle = InDevice->GetVulkanDynamicRHI()->GetSurfaceWindow();

		SwapChainSupportDetails swapChainSupport = InDevice->GetVulkanDynamicRHI()->QuerySwapChainSupport(InDevice->GetGPU());

		// KarmaGui may have, MAY, different requirements.
		m_SurfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
		m_PresentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);// Analogous to v-sync

		VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

		uint32_t imageCount = InDevice->GetVulkanDynamicRHI()->SwapChainImageCount();

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = InDevice->GetVulkanDynamicRHI()->GetSurface();
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = m_SurfaceFormat.format;
		createInfo.imageColorSpace = m_SurfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = InDevice->GetVulkanDynamicRHI()->FindQueueFamilies(InDevice->GetGPU());
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(),
			indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = m_PresentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		VkResult result = vkCreateSwapchainKHR(InDevice->GetLogicalDevice(), &createInfo, nullptr, &m_SwapChain);

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create swapchain!");
		KR_CORE_INFO("Created a Vulkan swapchain");

		vkGetSwapchainImagesKHR(InDevice->GetLogicalDevice(), m_SwapChain, &imageCount, nullptr);
		m_SwapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(InDevice->GetLogicalDevice(), m_SwapChain, &imageCount, m_SwapChainImages.data());

		m_SwapChainImageFormat = m_SurfaceFormat.format;
		m_SwapChainExtent = extent;
		
		m_SwapChainImageViews.resize(m_SwapChainImages.size());

		for (size_t i = 0; i < m_SwapChainImages.size(); i++)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = m_SwapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = m_SwapChainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			VkResult result = vkCreateImageView(InDevice->GetLogicalDevice(), &createInfo, nullptr, &m_SwapChainImageViews[i]);

			KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create image views!");
		}
	}

	void FVulkanSwapChain::Destroy(FVulkanSwapChainRecreateInfo * RecreateInfo)
	{
		m_Device->WaitUntilIdle();
		for (auto imageView : m_SwapChainImageViews)
		{
			vkDestroyImageView(m_Device->GetLogicalDevice(), imageView, nullptr);
		}
		
		vkDestroySwapchainKHR(m_Device->GetLogicalDevice(), m_SwapChain, nullptr);
	}

	VkSurfaceFormatKHR FVulkanSwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
				&& availableFormat.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR FVulkanSwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				KR_CORE_INFO("Vulkan found surface present mode: VK_PRESENT_MODE_MAILBOX_KHR");
				KR_CORE_INFO("Using VK_PRESENT_MODE_MAILBOX_KHR for swapchain present mode");

				return availablePresentMode;
			}
		}

		KR_CORE_INFO("Using the fallback VK_PRESENT_MODE_FIFO_KHR for swapchain");
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D FVulkanSwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			return capabilities.currentExtent;
		}
		else
		{
			int width, height;
			glfwGetFramebufferSize(m_WindowHandle, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::max(capabilities.minImageExtent.width,
				std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(capabilities.minImageExtent.height,
				std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}
}
