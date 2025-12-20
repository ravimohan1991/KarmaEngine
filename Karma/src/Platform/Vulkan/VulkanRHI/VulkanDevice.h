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

		///////////////// Utility Functions /////////////////
		/**
		 * @brief Transitions the layout of an image from oldLayout to newLayout.
		 *
		 * Image layout transitions are crucial in Vulkan to ensure that images are in the correct state for different operations, such as rendering, sampling, or transferring data.
		 *
		 * @param image							The image to be transitioned
		 * @param format						The format of the image
		 * @param oldLayout						The current layout of the image
		 * @param newLayout						The desired layout of the image
		 *
		 * @see VulkanTexture::CreateTextureImage()
		 * @since Karma 1.0.0
		 */
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

		/**
		 * @brief Copies data from a buffer to a Vulkan image.
		 *
		 * This is typically used for uploading texture data from a staging buffer to a Vulkan image.
		 *
		 * @param buffer						The source buffer containing the data
		 * @param image							The destination image
		 * @param width							The width of the image
		 * @param height						The height of the image
		 *
		 * @see VulkanTexture::CreateTextureImage()
		 * @since Karma 1.0.0
		 */
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

		/**
		 * @brief Checks if the given format has a stencil component.
		 *
		 * Sees if the format is VK_FORMAT_D32_SFLOAT_S8_UINT or VK_FORMAT_D24_UNORM_S8_UINT
		 *
		 * @param format							The format to be checked
		 *
		 * @see VulkanContext::TransitionImageLayout()
		 * @since Karma 1.0.0
		 */
		bool HasStencilComponent(VkFormat format);

		///////////////// Getters /////////////////

		/**
		 * @brief Getter for the graphics queue created in FVulkanDevice::InitGPU()
		 * 
		 * @since Karma 1.0.0
		 */
		inline VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }

	private:
		VkDevice m_LogicalDevice; ///< The Vulkan logical device handle.
		FVulkanDynamicRHI* m_VulkanDynamicRHI;

		VkPhysicalDevice m_GPU;
		VkQueue m_GraphicsQueue;
		VkQueue m_PresentQueue;
		VulkanTexture* m_DefaultTexture;

		// Additional members for managing queues, command pools, etc. can be added here.
		VkCommandPool m_CommandPool;
	};
}