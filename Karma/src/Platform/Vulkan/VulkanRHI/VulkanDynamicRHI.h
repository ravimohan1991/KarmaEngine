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

	/**
	 * @brief A structure for graphics and present queuefamilies
	 *
	 * Most operations performed with Vulkan, like draw commands and memory operations, are
	 * asynchronously executed by submitting them to a VkQueue. Queues are allocated from queue
	 * families, where each queue family supports a specific set of operations in its queues. For example,
	 * there could be separate queue families for graphics, compute and memory transfer operations.
	 *
	 * Used for creating logical device, swapchain, and commandpool
	 *
	 * @see FVulkanDynamicRHI::FindQueueFamilies()
	 * @since Karma 1.0.0
	 */
	struct QueueFamilyIndices
	{
		/**
		 * @brief The queues in this queue family support graphics operations.
		 *
		 * @note The optional is used to make the query of availibility easier
		 * @since Karma 1.0.0
		 */
		std::optional<uint32_t> graphicsFamily;

		/**
		 * @brief The queues in this queue family support image presentation
		 *
		 * The image is presented to the surface
		 *
		 * @note The optional is used to make the query of availibility easier
		 * @see VulkanContext::CreateSurface()
		 *
		 * @since Karma 1.0.0
		 */
		std::optional<uint32_t> presentFamily;

		/**
		 * @brief Routine for querying if appropriate queue families (graphicsFamily and presentFamily) are available.
		 *
		 * @see VulkanContext::IsDeviceSuitable
		 * @since Karma 1.0.0
		 */
		bool IsComplete()
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	/**
	 * @brief Structure with data required for appropriate creation and working of swapchain.
	 *
	 * Vulkan does not have the concept of a "default framebuffer", hence it requires an infrastructure that will own
	 * the buffers we will render to before we visualize them on the screen. This infrastructure is known as the swap chain
	 * and must be created explicitly in Vulkan. The swap chain is essentially a queue of images that are waiting to be
	 * presented to the screen.
	 *
	 * @since Karma 1.0.0
	 */
	struct SwapChainSupportDetails
	{
		/**
		 * @brief Basic surface capabilities (min/max number of images in swap chain, min/max width
		 * and height of images)
		 *
		 * @since Karma 1.0.0
		 */
		VkSurfaceCapabilitiesKHR capabilities;

		/**
		 * @brief Surface formats (pixel format, color space)
		 *
		 * @since Karma 1.0.0
		 */
		std::vector<VkSurfaceFormatKHR> formats;

		/**
		 * @brief Available presentation modes
		 *
		 * @since Karma 1.0.0
		 */
		std::vector<VkPresentModeKHR> presentModes;
	};

	class FVulkanDynamicRHI : public IVulkanDynamicRHI
	{
	public:
		/**
		 * @brief Getter for the FVulkanDynamicRHI instance
		 * 
		 * @since Karma 1.0.0
		 */
		static FVulkanDynamicRHI& Get() { return *GetDynamicRHI<FVulkanDynamicRHI>(); }

		/**
		 * @brief Initialization constructor.
		 * 
		 * Does the following
		 *		1. Creates vkInstance
		 *		2. Sets up debug messenger (with Karma's logging callback function)
		 *		3. Creates a Vulkan surface to interface with the Engine's window (Editor window for instance)
		 *		4. Selects a physical device (GPU) based upon the availability of device extensions, swapchain support, 
		 *		   and samplerAnisotropy
		 *		5. Creates Vulkan logical device
		 * 
		 * @note Smapler anisotorpy is a technique that makes textures on surfaces viewed at steep angles (like roads or floors
		 * stretching into the distance) look clearer and sharper, preventing blurriness by taking multiple samples along the 
		 * texture's axis of elongation rather than a single one.
		 * 
		 * @since Karma 1.0.0
		 */
		FVulkanDynamicRHI();

		/**
		 * @brief For initializing VulkanDynamic RHI
		 * 
		 * Called from RHIInit() declared in KarmaRHI.h and defined in DynamicRHI.cpp
		 * 
		 * @see FVulkanDynamicRHI::InitInstance()
		 * 
		 * @since Karma 1.0.0
		 */
		virtual bool Init() override;

		/**
		 * @brief Shuts down the RHI.
		 * 
		 * Called from RHIExit() declared in KarmaRHI.h and defined in DynamicRHI.cpp
		 * 
		 * Cleans up resources and states used by the RHI.
		 * @since Karma 1.0.0
		 */
		virtual void Shutdown() override;

		/**
		 * @brief Calls FVulkanDevice::InitGPU() to create Vulkan logical device from 
		 * selected GPU, and created appropriate resources (commandpool, default texture etc).
		 * 
		 * @see FVulkanDevice::InitGPU()
		 * @since Karma 1.0.0
		 */
		void InitInstance();

		/**
		 * @brief Presents the rendered frame to the display.
		 * 
		 * @since Karma 1.0.0
		 */
		virtual void Present() override {}

		/**
		 * @brief Getter for Vulkan instance
		 * 
		 * @since Karma 1.0.0
		 */
		inline VkInstance GetInstance() const
		{
			return m_VulkanInstance;
		}

		/**
		 * @brief Getter for FVulkanDevice object created in FVulkanDynamicRHI::SelectDevice()
		 * 
		 * @since Karma 1.0.0
		 */
		inline FVulkanDevice* GetDevice() const
		{
			return m_Device;
		}

		/**
		 * @brief Queries the graphics card for available queue families and compares against the availability of graphics and
		 * presentation queues
		 *
		 * @param device						The graphics card to be queired for queue family
		 * @since Karma 1.0.0
		 */
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
		
		/**
		 * @brief Getter for the boolean bEnableValidationLayers
		 * 
		 * For Debug builds bEnableValidationLayers is set to true
		 * 
		 * @since Karma 1.0.0
		 */
		inline bool GetValidationLayersSetting() const { return bEnableValidationLayers; }

		/**
		 * @brief Getter for m_SupportedDeviceFeatures which contains boolean values for supported features like
		 * availability of geometry shader or sampler anisotropy
		 * 
		 * @since Karma 1.0.0
		 */
		inline const VkPhysicalDeviceFeatures& GetGpuDeviceFeatures() const{ return m_SupportedDeviceFeatures; }

		/**
		 * @brief Getter for m_GPUSwapChainSupport struct which gets initialized in FVulkanDynamicRHI::IsDeviceSuitable()
		 * 
		 * @since Karma 1.0.0
		 */
		inline const SwapChainSupportDetails& GetGpuSwapChainSupportDetails() const { return m_GPUSwapChainSupport; }

		/**
		 * @brief Getter for the number of supported swapchain images
		 *
		 * @since Karma 1.0.0
		 */
		inline uint32_t SwapChainImageCount() const { return m_SwapChainImageCount; }

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

		/**
		 * @brief Picks an Engine appropriate GPU and creates FVulkanDevice
		 * 
		 * @since Karma 1.0.0
		 */
		void SelectDevice();

	private:

		/**
		 * @brief Computes the number of images supported by the GPU for swapchain creation
		 * 
		 * Makes sure that the m_SwapChainImageCount remains is in the interval [capabilities.minImageCount + 1, 
		 * capabilities.maxImageCount].
		 * 
		 * @since Karma 1.0.0
		 */
		void ComputeNumberOfSwapchainImagesSupported();

		/**
		 * @brief Uses Two-Pass Query to gather surface formats (physical device and surface paired color space or pixel format data) and present modes data.
		 *
		 * @see VulkanContext::IsDeviceSuitable(), and VulkanContext::CreateSwapChain()
		 * @since Karma 1.0.0
		 */
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

		/**
		 * @brief Looks for extension properties supported by the GPU
		 *
		 * Calls vkEnumerateDeviceExtensionProperties for list of supported extensions for instance VK_KHR_swapchain which is
		 * required for, well, swapchain
		 * 
		 * @note These are different from instance extensions printed in PrintAvailableExtensions()
		 *
		 * @since Karma 1.0.0
		 */
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

		/**
		 * @brief Checks if the physical device (graphics card) is suitable for the Engine to use based on
		 * avaibality of required queue families (graphics and presentation), required device extensions (like VK_KHR_swapchain), sampler anisotropy support (which is
		 * anisotropic filtering support in samplers, allowing higher-quality texture sampling at oblique angles to reduce blurring and aliasing artifacts seen in standard bilinear filtering.)
		 *
		 * @param device						The graphics card to be checked for suitability
		 * @since Karma 1.0.0
		 */
		bool IsDeviceSuitable(VkPhysicalDevice device);

		/**
		 * @brief Prints all the available physical devices (graphics cards) supported by the system's Vulkan implementation.
		 *
		 * @param physicalDevices						The list of GPUs detected
		 * 
		 * @see FVulkanDynamicRHI::SelectDevice()
		 * @since Karma 1.0.0
		 */
		void PrintAvailablePhysicalDevices(const std::vector<VkPhysicalDevice>& physicalDevices);

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
		 * @brief Prints all the available instance extensions supported by the system's Vulkan implementation (VkInstance).
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

	public:
		// To be set manually by coder
		static std::vector<const char*> deviceExtensions;
		static const std::vector<const char*> validationLayers;

	protected:
		// Vulkan instance handle and other Vulkan-specific members can be declared here

		uint32_t m_APIVersion;
		VkInstance m_VulkanInstance;
		FVulkanDevice* m_Device;
		static bool bEnableValidationLayers;
		VkDebugUtilsMessengerEXT m_DebugMessenger;
		GLFWwindow* m_WindowHandle;
		VkSurfaceKHR m_Surface;
		VkPhysicalDeviceFeatures m_SupportedDeviceFeatures;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;// GPU
		SwapChainSupportDetails m_GPUSwapChainSupport;
		uint32_t m_SwapChainImageCount;
	};
}