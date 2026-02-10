#include "VulkanPipeline.h"
#include "VulkanDevice.h"

namespace Karma
{
	FVulkanPipelineStateCacheManager::FVulkanPipelineStateCacheManager(FVulkanDevice* InDevice) : m_Device(InDevice)
	{
	}

	FVulkanPipelineStateCacheManager::~FVulkanPipelineStateCacheManager()
	{
	}

	bool FVulkanPipelineStateCacheManager::CreateGfxPipeline()
	{
		return false;
	}

	FVulkanPipeline::FVulkanPipeline(FVulkanDevice* InDevice) :
		m_Device(InDevice), m_Pipeline(VK_NULL_HANDLE)
	{
	}

	FVulkanPipeline::~FVulkanPipeline()
	{
		if (m_Pipeline != VK_NULL_HANDLE)
		{
			vkDestroyPipeline(m_Device->GetLogicalDevice(), m_Pipeline, nullptr);
			m_Pipeline = VK_NULL_HANDLE;
		}
	}
}