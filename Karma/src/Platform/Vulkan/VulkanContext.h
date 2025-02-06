/**
 * @file VulkanContext.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains VulkanContext class
 * @version 1.0
 * @date Jan 1, 2021
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#define GLFW_INCLUDE_VULKAN
#include "krpch.h"

#include "Karma/Renderer/GraphicsContext.h"
#include "GLFW/glfw3.h"
#include "vulkan/vulkan_core.h"
#include "Platform/Vulkan/VulkanBuffer.h"
#include "Platform/Vulkan/VulkanRendererAPI.h"

namespace Karma
{
	/**
	 * @brief Forward declaration
	 */
	class RendererAPI;

	/**
	 * @brief Forward declaration
	 */
	class VulkanVertexArray;

	/**
	 * @brief Forward declaration
	 */
	struct VulkanUniformBuffer;

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
	 * @see VulkanContext::FindQueueFamilies
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

	/**
	 * @brief Vulkan API has the following concepts
	 * 1. Physical Device (https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families): The software counterpart (VkPhysicalDevice) of a graphics card (GPU). Logical device is created from physical device.
	 * 2. Device (https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Logical_device_and_queues): The so called logical device for interfacing with the physical device. All the machinery (swapchain, graphicspipeline, and all that) are created from logical device.
	 *
	 * Host : is CPU the host?
	 */
	class KARMA_API VulkanContext : public GraphicsContext
	{
	public:
		/**
		 * @brief A constructor to set the m_vulkanRendererAPI (using static_cast, or compilet time cast).
		 * Also checks the validity of windowHandle.
		 *
		 * @param windowHandle								The glfw window handle
		 *
		 * @since Karma 1.0.0
		 */
		VulkanContext(GLFWwindow* windowHandle);
		
		/**
		 * @brief Destructor of vulkan context. Does the following
		 * 1. Free the commandbuffers (VulkanRendererAPI::AllocateCommandBuffers()) and removes synchronicity
		 * 2. Destroy the framebuffers (CreateFrameBuffers())
		 * 3. Destroy depth imageview (CreateDepthResources())
		 * 4. Destroy image (CreateDepthResources())
		 * 5. Free up depthimagememory (CreateDepthResources())
		 * 6. Destroy command pool (CreateCommandPool())
		 * 7. Destroy render pass (CreateRenderPass())
		 * 8. Destroy swapchain imageview (CreateImageViews())
		 * 9. Destroy swapchain (CreateSwapChain())
		 * 10. Destroy the vulkan m_device (CreateLogicalDevice())
		 * 11. Destroy validation layers for debug messages (SetupDebugMessenger())
		 * 12. Destroy surface (CreateSurface())
		 * 13. Destroy instance (CreateInstance())
		 */
		virtual ~VulkanContext() override;

		virtual void Init() override;
		virtual void SwapBuffers() override;
		virtual bool OnWindowResize(WindowResizeEvent& event) override {/*No need for Vulkan for now.*/ return true; }

		void CreateInstance();

		void PrintAvailableExtensions();
		void PrintAvailablePhysicalDevices(const std::vector<VkPhysicalDevice>& physicalDevices);

		// Validation layers
		bool CheckValidationLayerSupport();
		std::vector<const char*> GetRequiredExtensions(VkInstanceCreateFlags& flagsToBeSet);

		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);

		void SetupDebugMessenger();
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
			const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkDebugUtilsMessengerEXT* pDebugMessenger);

		void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

		// Surface
		void CreateSurface();

		// Physical device
		void PickPhysicalDevice();
		bool IsDeviceSuitable(VkPhysicalDevice device);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

		// Logical device
		/**
		 * @brief The so called logical device for interfacing with the physical device. All the machinery (swapchain, graphicspipeline, and all that) are created from logical device.
		 * @since Karma 1.0.0
		 */
		void CreateLogicalDevice();

		// Swapchain
		/**
		 * @brief Vulkan does not have the concept of a "default framebuffer", hence it requires an infrastructure that will own the buffers we will render to before we visualize them on the screen. This infrastructure is known as the swap chain and must be created explicitly in Vulkan. The swap chain is essentially a queue of images that are waiting to be presented to the screen. Our backend will acquire such an image to draw to it, and then return it to the queue.
		 *
		 * @brief Karma 1.0.0
		 */
		void CreateSwapChain();
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		// Image views
		/**
		 * @brief An image view is quite literally a view into an image. It describes how to access the image and which part of the image to access, for example if it should be treated as a 2D texture depth texture without any mipmapping levels.
		 *
		 * @note Here we are creating depth images ?
		 * @since Karma 1.0.0
		 */
		void CreateImageViews();

		//  Renderer pass
		/**
		 * @brief A VkRenderPass is a Vulkan object that encapsulates the state needed to setup the “target” for rendering, and the state of the images we will be rendering to.
		 * @since Karma 1.0.0
		 */
		void CreateRenderPass();

		// Framebuffers
		/**
		 * @brief A framebuffer represents a collection of specific memory attachments that a render pass instance uses.
		 *
		 * @see KarmaGui_ImplVulkanH_ImageFrame::Framebuffer
		 */
		void CreateFrameBuffers();

		// CommandPool
		/**
		 * @brief Command pools are opaque objects that command buffer memory is allocated from, and which allow the implementation to amortize the cost of resource creation.
		 *
		 * @since Karma 1.0.0
		 */
		void CreateCommandPool();

		// DepthImage
		void CreateDepthResources();
		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		VkFormat FindDepthFormat();
		bool HasStencilComponent(VkFormat format);

		// Texture relevant
		//void CreateTextureImage(VulkanImageBuffer* vImageBuffer);
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		//void CreateTextureImageView();
		//void CreateTextureSampler();

		void RecreateSwapChain();
		void CleanupSwapChain();

		void SetVSync(bool bEnable);

		void Initializeglslang();
		void RegisterUBO(const std::shared_ptr<VulkanUniformBuffer>& ubo);
		void ClearUBO();
		void RecreateUBO();
		void UploadUBO(size_t frameIndex);

		// Getters
		VkDevice GetLogicalDevice() const { return m_device; }
		VkPhysicalDevice GetPhysicalDevice() const { return m_physicalDevice; }
		VkExtent2D GetSwapChainExtent() const { return m_swapChainExtent; }
		VkRenderPass GetRenderPass() const { return m_renderPass; }
		const std::vector<VkFramebuffer>& GetSwapChainFrameBuffer() const { return m_swapChainFrameBuffers; }
		VkSwapchainKHR GetSwapChain() const { return m_swapChain; }
		inline const std::vector<VkImage>& GetSwapChainImages() const { return m_swapChainImages; }
		VkFormat GetSwapChainImageFormat() const { return m_swapChainImageFormat; }
		const std::vector<VkImageView>& GetSwapChainImageViews() const { return m_swapChainImageViews; }
		VkSurfaceFormatKHR GetSurfaceFormat() const { return m_surfaceFormat; }
		VkQueue GetGraphicsQueue() const { return m_graphicsQueue; }
		VkQueue GetPresentQueue() const { return m_presentQueue; }
		VkCommandPool GetCommandPool() const { return m_commandPool; }
		//VkImageView GetTextureImageView() const { return m_TextureImageView; }
		//VkSampler GetTextureSampler() const { return m_TextureSampler; }
		const VkPhysicalDeviceFeatures& GetSupportedDeviceFeatures() const { return m_SupportedDeviceFeatures; }
		VkInstance GetInstance() const { return m_Instance; }
		uint32_t GetImageCount() const { return uint32_t(m_swapChainImages.size()); }
		uint32_t GetMinImageCount() const { return m_MinImageCount; }
		VkSurfaceKHR GetSurface() const { return m_surface; }
		VkPresentModeKHR GetPresentMode() const { return m_presentMode; }

	private:
		// Apologies for little out-of-sync naming convention, was dealing with flood of lines when
		// learning Vulkan!
		GLFWwindow* m_windowHandle;
		VulkanRendererAPI* m_vulkanRendererAPI;

		VkInstance m_Instance;
		VkDebugUtilsMessengerEXT debugMessenger;

		VkPhysicalDeviceFeatures m_SupportedDeviceFeatures;

		static bool bEnableValidationLayers;

		VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
		VkDevice m_device;
		VkQueue m_graphicsQueue;

		VkSurfaceKHR m_surface;
		VkQueue m_presentQueue;
		VkPresentModeKHR m_presentMode;

		VkSurfaceFormatKHR m_surfaceFormat;

		VkSwapchainKHR m_swapChain;
		std::vector<VkImage> m_swapChainImages;
		VkFormat m_swapChainImageFormat;
		VkExtent2D m_swapChainExtent;
		std::vector<VkImageView> m_swapChainImageViews;

		VkRenderPass m_renderPass;

		std::vector<VkFramebuffer> m_swapChainFrameBuffers;
		VkCommandPool m_commandPool;

		std::set<std::shared_ptr<VulkanUniformBuffer>> m_VulkanUBO;

		bool bVSync = false;

		VkImage m_DepthImage;
		VkDeviceMemory m_DepthImageMemory;
		VkImageView m_DepthImageView;

		uint32_t m_MinImageCount = 0;

		//VkImage m_TextureImage;
		/*
		VkDeviceMemory m_TextureImageMemory;
		VkImageView m_TextureImageView;
		VkSampler m_TextureSampler;
		 */
	};
}
