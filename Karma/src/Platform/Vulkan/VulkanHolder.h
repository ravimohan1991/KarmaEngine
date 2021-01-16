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

	private:
		static VkDevice* m_VulkanDevice;
		static VkPhysicalDevice* m_VulkanPhysicalDevice;
	};
}