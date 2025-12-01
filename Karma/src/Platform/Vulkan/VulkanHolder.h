/**
 * @file VulkanHolder.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains VulkanHolder class for holding VulkanContext instance statically.
 * @version 1.0
 * @date Jan 16, 2021
 * 
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "krpch.h"

#include "Platform/Vulkan/VulkanContext.h"

namespace Karma
{
	/**
	 * @brief A holder class to statically hold VulkanContext instance.
	 * 
	 * This is useful to access VulkanContext from anywhere in the codebase without needing to pass around references or pointers.
	 * 
	 * @since Karma 1.0.0
	 */
	class KARMA_API VulkanHolder
	{
	public:

		/**
		 * @brief Sets the VulkanContext instance.
		 * 
		 * @param vulkanContext					Pointer to the VulkanContext instance to be set.
		 * 
		 * @see VulkanContext::Init()
		 * @since Karma 1.0.0
		 */
		static void SetVulkanContext(VulkanContext* vulkanContext)
		{
			m_VulkanContext = vulkanContext;
		}

		/**
		 * @brief Gets the VulkanContext instance.
		 * 
		 * @return Pointer to the VulkanContext instance.
		 * 
		 * @since Karma 1.0.0
		 */
		static VulkanContext* GetVulkanContext()
		{
			return m_VulkanContext;
		}

	private:
		static VulkanContext* m_VulkanContext;
	};
}