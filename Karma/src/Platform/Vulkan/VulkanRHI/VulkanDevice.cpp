#include "VulkanDevice.h"
#include "VulkanDynamicRHI.h"

namespace Karma
{
	FVulkanDevice::FVulkanDevice(FVulkanDynamicRHI* InRHI, VkPhysicalDevice InGpu) : m_VulkanDynamicRHI(InRHI),
		m_GPU(InGpu)
	{
		// Implementation for creating a Vulkan logical device
	}

	FVulkanDevice::~FVulkanDevice()
	{
		// Implementation for cleaning up Vulkan device resources
	}

	void FVulkanDevice::Destroy()
	{
		// vkdestroy default buffers etc

		vkDestroyDevice(m_LogicalDevice, nullptr);
	}

	void FVulkanDevice::InitGPU()
	{
		QueueFamilyIndices indices = m_VulkanDynamicRHI->FindQueueFamilies(m_GPU);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(),
		indices.presentFamily.value() };

		if (m_VulkanDynamicRHI->GetValidationLayersSetting())
		{
			KR_CORE_INFO("+-------------------------------------------------");
			KR_CORE_INFO("| Available Unique Queue Family Indices (Graphics Card):");
			uint32_t index = 1;
			for (uint32_t queueFamily : uniqueQueueFamilies)
			{
				KR_CORE_INFO("| {0}. {1}", index++, queueFamily);
			}
			KR_CORE_INFO("+-------------------------------------------------");
		}

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		if (m_VulkanDynamicRHI->GetGpuDeviceFeatures().logicOp)
		{
			deviceFeatures.logicOp = VK_TRUE;
		}

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(FVulkanDynamicRHI::deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = FVulkanDynamicRHI::deviceExtensions.data();

		if (m_VulkanDynamicRHI->GetValidationLayersSetting())
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(FVulkanDynamicRHI::validationLayers.size());
			createInfo.ppEnabledLayerNames = FVulkanDynamicRHI::validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		VkResult result = vkCreateDevice(m_GPU, &createInfo, nullptr, &m_LogicalDevice);

		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create logical device!");

		vkGetDeviceQueue(m_LogicalDevice, indices.graphicsFamily.value(), 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_LogicalDevice, indices.presentFamily.value(), 0, &m_GraphicsQueue);
	}
}