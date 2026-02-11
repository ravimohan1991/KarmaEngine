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
#include "VulkanSynchronization.h"

namespace Karma
{
	class FVulkanDynamicRHI;
	class VulkanTexture;
	class FVulkanDescriptorSetsLayout;
	class FVulkanDescriptorSets;
	class FVulkanSwapChain;

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
		 * Initializes the Vulkan device with the given physical device (GPU) and dynamic RHI instance.
		 * 
		 * @param InRHI			Pointer to the FVulkanDynamicRHI instance
		 * @param InGpu			The Vulkan physical device (GPU) handle
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
		 * @brief Waits on the host by blocking the calling CPU thread until the 
		 * Vulkan logical device completes all pending GPU operations across all queues.
		 * 
		 * @since Karma 1.0.0
		 */
		void WaitUntilIdle();

		/**
		 * @brief Retrieves the Vulkan logical device handle.
		 * 
		 * @return VkDevice The Vulkan logical device.
		 */
		VkDevice GetLogicalDevice() const { return m_LogicalDevice; }

		/**
		 * @brief Retrieves the Vulkan physical device handle.
		 * 
		 * @return VkPhysicalDevice The Vulkan physical device (GPU)
		 */
		VkPhysicalDevice GetGPU() const { return m_GPU; }

		/**
		 * @brief Retrives the Vulkan command pool for the commandbuffers
		 * 
		 * @since Karma 1.0.0
		 */
		VkCommandPool GetCommandPool() const { return m_CommandPool; }

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

		void InitializeDefaultDescriptorSets(uint32_t MaxFramesInFlight);

		///////////////// Getters /////////////////

		/**
		 * @brief Getter for the graphics queue created in FVulkanDevice::InitGPU()
		 * 
		 * @since Karma 1.0.0
		 */
		inline VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }

		/**
		 * @brief Getter for the present queue created in FVulkanDevice::InitGPU()
		 * 
		 * @since Karma 1.0.0
		 */
		inline FVulkanDynamicRHI* GetVulkanDynamicRHI() const { return m_VulkanDynamicRHI; }

		/**
		 * @brief Getter for the default texture (unreal grid)
		 * 
		 * @since Karma 1.0.0
		 */
		inline FVulkanFenceManager& GetFenceManager() { return m_FenceManager; }

		inline KarmaVector<FVulkanDescriptorSets*>& GetDefaultDescriptorSets() { return m_DefaultDescriptorSets; }

	private:
		/**
		 * @brief Populates the provided descriptor sets layout with the necessary bindings for the device.
		 *
		 * This function is responsible for defining the descriptor set layouts that will be used for resource binding in shaders. It 
		 * adds the required bindings for global resources (like camera UBO and texture sampler) and per-mesh resources (like per-mesh UBO).
		 * 
		 * From shader POV, the sets are like so
		 * set = 0, binding = 0: Camera UBO (vertex shader)
		 * set = 0, binding = 1: Texture sampler (fragment shader)
		 * 
		 * set = 1, binding = 0: Per-mesh UBO (vertex and fragment shader)
		 *
		 * @param OutLayout						The descriptor sets layout to be populated with the necessary bindings.
		 *
		 * @see FVulkanDescriptorSetsLayout
		 * @since Karma 1.0.0
		 */
		void PopulateWithDescriptorSetsLayout(FVulkanDescriptorSetsLayout& OutLayout);
		
	private:
		VkDevice m_LogicalDevice; ///< The Vulkan logical device handle.
		FVulkanDynamicRHI* m_VulkanDynamicRHI;

		VkPhysicalDevice m_GPU;
		VkQueue m_GraphicsQueue;
		VkQueue m_PresentQueue;
		VulkanTexture* m_DefaultTexture;

		// Additional members for managing queues, command pools, etc. can be added here.
		VkCommandPool m_CommandPool;

		// Default descriptor set layout for the device, which can be used for common resources like camera UBO and default texture sampler.
		FVulkanDescriptorSetsLayout* m_DefaultDescriptorSetLayout;
		KarmaVector<class FVulkanDescriptorPool*> m_DescriptorPool;
		KarmaVector<FVulkanDescriptorSets*> m_DefaultDescriptorSets;
		uint32_t m_MaxFramesInFlight = 0;

		FVulkanFenceManager	m_FenceManager;
	};
}
