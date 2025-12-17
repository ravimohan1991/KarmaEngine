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

struct GLFWwindow;

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
		 * @brief Sets up the debug messenger for Vulkan instance.
		 *
		 * This function initializes the debug messenger that will handle debug messages from the Vulkan API.
		 * It populates the debug messenger create info structure and creates the debug messenger using
		 * the CreateDebugUtilsMessengerEXT function.
		 *
		 * @see FVulkanDynamicRHI::CreateDebugUtilsMessengerEXT()
		 * @since Karma 1.0.0
		 */
		void SetupDebugMessenger();

		/**
		 * @brief Platform agnostic creation of surface to present rendered images to. Typically they are backed by the 
		 * category of glfw windows (on Linux, MacOS, and Windows).
		 *
		 * @since Karma 1.0.0
		 */
		void CreateSurface();

		void SelectDevice();

	private:

		/**
		 * @brief Creates a debug utils messenger for Vulkan instance.
		 *
		 * This function is used to set up a debug messenger that will handle debug messages from the Vulkan API.
		 * Basically looks for the vkCreateDebugUtilsMessengerEXT extension, and if found, uses it to create the debug messenger by
		 * creating function pointer to it using vkGetInstanceProcAddr.
		 *
		 * @param instance			The Vulkan instance.
		 * @param pCreateInfo		Pointer to the debug utils messenger create info structure.
		 * @param pAllocator		Optional custom allocator.
		 * @param pDebugMessenger	Pointer to the debug utils messenger handle to be created.
		 *
		 * @return VkResult indicating success or failure of the operation.
		 *
		 * @see FVulkanDynamicRHI::SetupDebugMessenger()
		 * @since Karma 1.0.0
		 */
		VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
			const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkDebugUtilsMessengerEXT* pDebugMessenger);

		/**
		 * @brief Destroys the debug utils messenger for Vulkan instance.
		 * 
		 * @param instance			The Vulkan instance.
		 * @param debugMessenger	The debug utils messenger to be destroyed.
		 * 
		 * @param pAllocator		Optional custom allocator.
		 * 
		 * @since Karma 1.0.0
		 */
		void DestroyDebugUtilsMessengerEXT(VkInstance instance, 
			VkDebugUtilsMessengerEXT debugMessenger, 
			const VkAllocationCallbacks* pAllocator);

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
		 * Basically for choosing which type of debug messages we want to be displayed
		 * 
		 * @param createInfo		Reference to the VkDebugUtilsMessengerCreateInfoEXT structure to be populated.
		 *
		 * @note This function sets DebugCallback as the callback function for handling (printing, for instance) debug messages.
		 * @see FVulkanDynamicRHI::SetupDebugMessenger(), FVulkanDynamicRHI::CreateInstance()
		 *
		 * @since Karma 1.0.0
		 */
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

		/**
		 * @brief Callback function for Vulkan debug messages.
		 * 
		 * Here we define the actions to be taken, by the Engine, when a debug message is received from the Vulkan API.
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
		VkDebugUtilsMessengerEXT m_DebugMessenger;
		GLFWwindow* m_WindowHandle;
		VkSurfaceKHR m_Surface;
	};
}