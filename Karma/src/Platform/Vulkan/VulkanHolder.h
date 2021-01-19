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

	private:
		static VulkanContext* m_VulkanContext;
	};
}