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
		 * Initializes the Vulkan device with the specified physical device and instance.
		 * 
		 * @param physicalDevice		The Vulkan physical device to create the logical device from.
		 * @param instance				The Vulkan instance associated with this device.
		 * 
		 * @since Karma 1.0.0
		 */
		FVulkanDevice(VkPhysicalDevice physicalDevice, VkInstance instance);

		/**
		 * @brief Destructor for FVulkanDevice.
		 * 
		 * Cleans up resources associated with the Vulkan device.
		 * 
		 * @since Karma 1.0.0
		 */
		~FVulkanDevice();

		/**
		 * @brief Retrieves the Vulkan logical device handle.
		 * 
		 * @return VkDevice The Vulkan logical device.
		 */
		VkDevice GetLogicalDevice() const { m_LogicalDevice; }

	private:
		VkDevice m_LogicalDevice; ///< The Vulkan logical device handle.
		// Additional members for managing queues, command pools, etc. can be added here.
	};
}