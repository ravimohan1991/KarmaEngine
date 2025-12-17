/**
 * @file VulkanDynamicRHI.h
 * @brief
 * @author Ravi Mohan (the_cowboy)
 * @version 1.0
 * @date December 15, 2025
 * 
 * @copyright Karma Engine copyright(c) People of India
 */

#pragma once

#include "DynamicRHI.h"
#include "VulkanRHI.h"

#include "VulkanDevice.h"

namespace Karma
{
	template<typename TRHI>
	FORCEINLINE TRHI* GetDynamicRHI()
	{
		return static_cast<TRHI*>(GDynamicRHI);
	}

	class FVulkanDynamicRHI : public IVulkanDynamicRHI
	{
	public:
		static FVulkanDynamicRHI& Get() { return *GetDynamicRHI<FVulkanDynamicRHI>(); }

		/** Initialization constructor. */
		FVulkanDynamicRHI();

		virtual bool Init() override;
		/**
		 * @brief Shuts down the RHI.
		 *
		 * Cleans up resources and states used by the RHI.
		 */
		virtual void Shutdown() override;

		/**
		 * @brief Presents the rendered frame to the display.
		 * 
		 * @since Karma 1.0.0
		 */
		virtual void Present() override {}

		inline VkInstance GetInstance() const
		{
			return m_VulkanInstance;
		}

		inline FVulkanDevice* GetDevice() const
		{
			return m_Device;
		}

	protected:
		// Vulkan-specific members and methods can be added here

		/**
		 * @brief Creates a Vulkan instance.
		 * 
		 * @since Karma 1.0.0
		 */
		void CreateInstance();

		/**
		 * @brief Prints all the available extensions supported by the system's Vulkan implementation (VkInstance).
		 *
		 * Vulkan extensions provide additional functionality beyond the core Vulkan API.
		 * 
		 * For instance VK_KHR_Surface (for window system integration) or VK_KHR_get_physical_device_properties2 (for querying extended physical device properties).
		 *
		 * @see VulkanContext::CreateInstance()
		 * @since Karma 1.0.0
		 */
		void PrintAvailableExtensions();

		/**
		 * @brief Retrieves the required Vulkan instance extensions based on the platform and validation layer settings.
		 * 
		 * 1. For window system integration, queries GLFW for required extensions.
		 * 2. For MacOS, adds VK_KHR_portability_enumeration extension if available.
		 * 
		 * @param flagsToBeSet		Reference to VkInstanceCreateFlags to be set based on required extensions.
		 * 
		 * @return A vector of required extension names.
		 * 
		 * @see VulkanContext::CreateInstance()
		 * @since Karma 1.0.0
		 */
		std::vector<const char*> GetRequiredExtensions(VkInstanceCreateFlags& flagsToBeSet);

		/**
		 * @brief Checks if the requested validation layers are supported by the Vulkan implementation.
		 * 
		 * Validation layers are used for debugging and development purposes to catch errors and provide additional information during Vulkan API usage.
		 * 
		 * @return true if all requested validation layers are supported, false otherwise.
		 * 
		 * @since Karma 1.0.0
		 */
		bool CheckValidationLayerSupport();

		/**
		 * @brief Populates the debug messenger create info structure for setting up validation layers.
		 * 
		 * @param createInfo		Reference to the VkDebugUtilsMessengerCreateInfoEXT structure to be populated.
		 * 
		 * @note This function sets DebugCallback as the callback function for handling (printing, for instance) debug messages.
		 * @see DebugCallback()
		 * 
		 * @since Karma 1.0.0
		 */
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

		/**
		 * @brief Callback function for Vulkan debug messages.
		 * 
		 * @param messageSeverity		Severity of the debug message.
		 * @param messageType			Type of the debug message.
		 * @param pCallbackData			Pointer to the callback data containing the debug message.
		 * @param pUserData				User-defined data passed to the callback.
		 * 
		 * @since Karma 1.0.0
		 */
		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);

	protected:
		// Vulkan instance handle and other Vulkan-specific members can be declared here

		uint32_t m_APIVersion;
		VkInstance m_VulkanInstance;

		FVulkanDevice* m_Device;
		static bool bEnableValidationLayers;
	};
}