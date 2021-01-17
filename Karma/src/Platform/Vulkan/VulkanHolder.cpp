#include "VulkanHolder.h"

namespace Karma
{
	VkDevice* VulkanHolder::m_VulkanDevice = nullptr;
	VkPhysicalDevice* VulkanHolder::m_VulkanPhysicalDevice = nullptr;
	VkExtent2D* VulkanHolder::m_swapChainExtent = nullptr;
	VkRenderPass* VulkanHolder::m_renderPass = nullptr;
	VkPipeline* VulkanHolder::m_graphicsPipeline = nullptr;
}