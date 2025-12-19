/**
 * @file VulkanDevice.h
 * @brief Declaration of the FVulkanDevice class for managing Vulkan device resources.
 * @author Ravi Mohan (the_cowboy)
 * @version 1.0
 * @date December 17, 2025
 * 
 * @copyright Karma Engine copyright(c) People of India
 */

#pragma once

#include <vulkan/vulkan.h>

namespace Karma
{
	class FVulkanDynamicRHI;
	class VulkanTexture;

	/**
	 * @class FVulkanDevice
	 * @brief Manages Vulkan device resources and operations.
	 * 
	 * This class encapsulates the functionality related to a Vulkan logical device,
	 * including initialization, resource management, and cleanup.
	 */
	class FVulkanDevice
	{
	public:
		/**
		 * @brief Constructor for FVulkanDevice.
		 * 
		 *
		 * 
		 * @param 
		 * 
		 * @since Karma 1.0.0
		 */
		FVulkanDevice(FVulkanDynamicRHI* InRHI, VkPhysicalDevice InGpu);

		/**
		 * @brief Destructor for FVulkanDevice.
		 * 
		 * Cleans up resources associated with the Vulkan device.
		 * 
		 * @since Karma 1.0.0
		 */
		~FVulkanDevice();

		/**
		 * @brief Creates the Vulkan logical device and default vulkan resources
		 * 
		 * @note Called from FVulkanDynamicRHI::InitInstance()
		 * @since Karma 1.0.0
		 */
		void InitGPU();

		/**
		 * @brief Destroys the logical vkdevice by making Vulkan API call
		 * 
		 * @since Karma 1.0.0
		 */
		void Destroy();

		/**
		 * @brief Retrieves the Vulkan logical device handle.
		 * 
		 * @return VkDevice The Vulkan logical device.
		 */
		VkDevice GetLogicalDevice() const { return m_LogicalDevice; }

		/**
		 * @biref Retrieves the Vulkan physical device handle.
		 * 
		 * @return VkPhysicalDevice The Vulkan physical device (GPU)
		 */
		VkPhysicalDevice GetGPU() const { return m_GPU; }

	private:
		VkDevice m_LogicalDevice; ///< The Vulkan logical device handle.
		FVulkanDynamicRHI* m_VulkanDynamicRHI;

		VkPhysicalDevice m_GPU;
		VkQueue m_GraphicsQueue;

		VulkanTexture* m_DefaultTexture;

		// Additional members for managing queues, command pools, etc. can be added here.
	};
}