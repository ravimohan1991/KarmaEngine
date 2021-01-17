#pragma once

#include "Karma/Core.h"
#include "Vulkan/vulkan_core.h"

namespace Karma
{
	class KARMA_API VulkanHolder
	{
	public:
		static void SetVulkanDevice(VkDevice* device)
		{
			m_VulkanDevice = device;
		}

		static VkDevice* GetVulkanDevice()
		{
			return m_VulkanDevice;
		}

		static void SetVulkanPhysicalDevice(VkPhysicalDevice* physicalDevice)
		{
			m_VulkanPhysicalDevice = physicalDevice;
		}

		static VkPhysicalDevice* GetVulkanPhysicalDevice()
		{
			return m_VulkanPhysicalDevice;
		}

		static void SetVulkanSwapChainExtent(VkExtent2D* swapChainExtent)
		{
			m_swapChainExtent = swapChainExtent;
		}

		static VkExtent2D* GetVulkanSwapChainExtent()
		{
			return m_swapChainExtent;
		}

		static void SetVulkanRenderPass(VkRenderPass* renderPass)
		{
			m_renderPass = renderPass;
		}

		static VkRenderPass* GetVulkanRenderPass()
		{
			return m_renderPass;
		}

		static void SetVulkanPipeline(VkPipeline* pipeline)
		{
			m_graphicsPipeline = pipeline;
		}

		static VkPipeline* GetVulkanPipeline()
		{
			return m_graphicsPipeline;
		}

	private:
		static VkDevice* m_VulkanDevice;
		static VkPhysicalDevice* m_VulkanPhysicalDevice;

		static VkExtent2D* m_swapChainExtent;
		static VkRenderPass* m_renderPass;

		static VkPipeline* m_graphicsPipeline;
	};
}