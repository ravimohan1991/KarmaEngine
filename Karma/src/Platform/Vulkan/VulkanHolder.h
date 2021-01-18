#pragma once

#include "Karma/Core.h"
#include "Platform/Vulkan/VulkanContext.h"

namespace Karma
{
	class KARMA_API VulkanHolder
	{
	public:
		static void SetVulkanContext(VulkanContext* vulkanContext)
		{
			m_VulkanContext = vulkanContext;
		}

		static VulkanContext* GetVulkanContext()
		{
			return m_VulkanContext;
		}

		static void SetVulkanPipeline(VkPipeline pipeline)
		{
			m_Pipeline = pipeline;
		}

		static VkPipeline GetVulkanPipeline()
		{
			return m_Pipeline;
		}

	private:
		static VulkanContext* m_VulkanContext;
		static VkPipeline m_Pipeline;
	};
}