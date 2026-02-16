/**
 * @file VulkanPipeline.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains FVulkanPipeline class and relevant data structures.
 * @version 1.0
 * @date February 6, 2026
 * 
 * @copyright Karma Engine copyright(c) People of India
 */

#pragma once

#include <vulkan/vulkan.h>

namespace Karma
{
	class FVulkanDevice;

	class FVulkanPipelineStateCacheManager
	{
	public:
		FVulkanPipelineStateCacheManager(FVulkanDevice* InDevice);
		~FVulkanPipelineStateCacheManager();

	private:
		bool CreateGfxPipeline();

	private:
		FVulkanDevice* m_Device;

	};

	class FVulkanPipeline
	{
	public:
		FVulkanPipeline(FVulkanDevice* InDevice);

		/*virtual*/ ~FVulkanPipeline();

	protected:
		FVulkanDevice* m_Device;
		VkPipeline m_Pipeline;

		//FVulkanLayout* m_Layout;
	};
}