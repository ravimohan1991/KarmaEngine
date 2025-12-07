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
	 * @see VulkanContext::FindQueueFamilies()
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
		 * 14. Destroy glslang memory resources for cleanup
		 *
		 * @see Init()
		 * @since Karma 1.0.0
		 */
		virtual ~VulkanContext() override;

		/**
		 * @brief Initializes VulkanContext by creating appropriate Vulkan and glslang specific
		 * instruments and allocating resources accordingly.
		 *
		 * 1. Create Instance;
		 * 2. Setup Debug Messenger
		 * 3. Create Surface
		 * 4. Pick PhysicalDevice
		 * 5. Create Logical Device
		 * 6. Create Swap Chain
		 * 7. Create ImageViews
		 * 8. Create RenderPass
		 * 9. Create CommandPool
		 * 10. Create DepthResources
		 * 11. Create FrameBuffers
		 * 12. VulkanHolder::SetVulkanContext(this) (VulkanHolder::m_VulkanContext)
		 * 13. m_vulkanRendererAPI->CreateSynchronicity()
		 * 14. Initialize glslang()
		 *
		 * @see ~VulkanContext()
		 * @since Karma 1.0.0
		 */
		virtual void Init() override;

		/**
		 * @brief In Vulkan there is no default framebuffer, hence we need to explicitly swap the buffers in the swapchain.
		 * 
		 * For instance, acquire an image from the swapchain, execute the command buffer with that image as attachment in the framebuffer, and return the image to the swapchain for presentation.
		 * All these are done in VulkanRendererAPI::SubmitCommandBuffers(). So no need to do anything here.
		 * 
		 * @see VulkanRendererAPI::SubmitCommandBuffers()
		 * @since Karma 1.0.0
		 */
		virtual void SwapBuffers() override;

		virtual bool OnWindowResize(WindowResizeEvent& event) override {/*No need for Vulkan for now.*/ return true; }

		/**
		 * @brief Creates Vulkan Instance which is the connection between application, which is a Game Engine, and Vulkan library.
		 * 
		 * @since Karma 1.0.0
		 */
		void CreateInstance();

		/**
		 * @brief Prints all the available extensions supported by the system's Vulkan implementation.
		 * 
		 * For instance VK_KHR_Surface or VK_KHR_get_physical_device_properties2.
		 * 
		 * @see VulkanContext::CreateInstance()
		 * @since Karma 1.0.0
		 */
		void PrintAvailableExtensions();

		/**
		 * @brief Prints all the available physical devices (graphics cards) supported by the system's Vulkan implementation.
		 * 
		 * @see VulkanContext::CreateInstance()
		 * @since Karma 1.0.0
		 */
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

		/**
		 * @brief Platform agnostic creation of surface to present rendered images to. Typically they are backed by the category of glfw windows (on Linux, MacOS, and Windows).
		 *
		 * @since Karma 1.0.0
		 */
		void CreateSurface();

		/**
		 * @brief Picks the appropriate physical device (graphics card) for the Engine to use based on suitability.
		 * 
		 * 
		 * @see VulkanContext::IsDeviceSuitable()
		 * @since Karma 1.0.0
		 */
		void PickPhysicalDevice();

		/**
		 * @brief Checks if the physical device (graphics card) is suitable for the Engine to use based on
		 * 
		 * avaibality of required queue families (graphics and presentation), required device extensions (like VK_KHR_swapchain), sampler anisotropy support (which is 
		 * anisotropic filtering support in samplers, allowing higher-quality texture sampling at oblique angles to reduce blurring and aliasing artifacts seen in standard bilinear filtering.)
		 *
		 * @param device						The graphics card to be checked for suitability
		 * @since Karma 1.0.0
		 */
		bool IsDeviceSuitable(VkPhysicalDevice device);
		
		/**
		 * @brief Queries the graphics card for available queue families and compares against the availability of graphics and presentation queues
		 *
		 * @param device						The graphics card to be queired for queue family
		 * @since Karma 1.0.0
		 */
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

		/**
		 * @brief The so called logical device for interfacing with the physical device. All the machinery (swapchain, graphicspipeline, and all that) are created from logical device. Following is done:
		 * 1. Inquires the graphics card for available queue families (FindQueueFamilies)
		 * 2. Create queues from both graphics and presentation families
		 * The logical device (VkDevice) m_device is created with the above features
		 *
		 * @note Various Vulkan operations from graphics to compute are asynchronously executed by submitting them to the queues having different cababilities depending on the queue family they are derived from.
		 *
		 * @see VulkanContext::FindQueueFamilies()
		 * @since Karma 1.0.0
		 */
		void CreateLogicalDevice();

		/**
		 * @brief Vulkan does not have the concept of a "default framebuffer", hence it requires an infrastructure that will own the buffers we will render to before we visualize them on the screen. This infrastructure is known as the swap chain and must be created explicitly in Vulkan. The swap chain is essentially a queue of images that are waiting to be presented to the screen. Our backend will acquire such an image to draw to it, and then return it to the queue. Then swapchain will have to sync the presentation of image with refresh rate of the monitor.
		 *
		 * 1. Sets the size of m_swapChainImages based on supported number of images by the graphics card
		 * 2. Sets m_surfaceFormat and m_presentMode
		 *
		 * @note Requires the availability of Requires VK_KHR_swapchain extension which is queried in IsDeviceSuitable() (which calls 
		 * CheckDeviceExtensionSupport()).
		 *
		 * @see VulkanContext::ChooseSwapExtent(), VulkanContext::QuerySwapChainSupport()
		 * @since Karma 1.0.0
		 */
		void CreateSwapChain();

		/**
		 * @brief Looks for extension properties supported by the GPU
		 * 
		 * Calls vkEnumerateDeviceExtensionProperties for list of supported extensions for instance VK_KHR_swapchain which is
		 * required for, well, swapchain
		 * 
		 * @
		 */
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		
		/**
		 * @brief Uses Two-Pass Query to gather surface formats (physical device and surface paired color space or pixel format data) and present modes data.
		 *
		 * @see VulkanContext::IsDeviceSuitable(), and VulkanContext::CreateSwapChain()
		 * @since Karma 1.0.0
		 */
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

		/**
		 * @brief Chooses the best surface format (pixel format and color space) for the swapchain from the available formats.
		 * 
		 * Basically looks for VK_FORMAT_B8G8R8A8_SRGB and VK_COLOR_SPACE_SRGB_NONLINEAR_KHR combination.
		 * 
		 * VK_FORMAT_B8G8R8A8_SRGB : represents a 32-bit format with 8 bits for each of the blue, green, red, and alpha channels in sRGB color space. 
		 * This format is widely used for swapchain images and color attachments.
		 * 
		 * VK_COLOR_SPACE_SRGB_NONLINEAR_KHR : represents the sRGB color space with a nonlinear gamma curve. This color space is commonly used for displaying images on standard monitors.
		 * 
		 * @param availableFormats						The available surface formats (from QuerySwapChainSupport())
		 * 
		 * @see VulkanContext::CreateSwapChain()
		 * @since Karma 1.0.0
		 */
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

		/**
		 * @brief Chooses the best presentation mode for the swapchain from the available present modes.
		 * 
		 * Basically looks for VK_PRESENT_MODE_MAILBOX_KHR (triple buffering) first, then VK_PRESENT_MODE_IMMEDIATE_KHR (tearing possible), and finally defaults to VK_PRESENT_MODE_FIFO_KHR (always available, v-sync)
		 * 
		 * @param availablePresentModes				The available presentation modes (from QuerySwapChainSupport())
		 * 
		 * @see VulkanContext::CreateSwapChain()
		 * @since Karma 1.0.0
		 */
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		
		/**
		 * @brief Chooses the swap extent (resolution of the swapchain images) based on the capabilities of the surface and the actual window size.
		 * 
		 * @param capabilities						The surface capabilities (from QuerySwapChainSupport())
		 * 
		 * @see VulkanContext::CreateSwapChain()
		 * @since Karma 1.0.0
		 */
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		/**
		 * @brief Finds a suitable memory type on the physical device (graphics card) based on the type filter and desired properties.
		 *
		 * @param typeFilter						Bitmask specifying the acceptable memory types
		 * @param properties						Desired memory properties (like VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT for CPU access)
		 *
		 * @see VulkanBuffer::VulkanBuffer()
		 * @since Karma 1.0.0
		 */
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		/**
		 * @brief An image view is quite literally a view into an image. It describes how to access the image and which part of the image to access, for example if it should be treated as a 2D texture depth texture without any mipmapping levels. ImageView is a wrapper around render target.
		 *
		 * @note Here we are creating depth images ? Basically depth is taken care of by depth attachment render target (created in VulkanContext::CreateRenderPass())
		 * @since Karma 1.0.0
		 */
		void CreateImageViews();

		/**
		 * @brief A VkRenderPass is a Vulkan object that encapsulates the state needed to setup the “target” for rendering, and the state of the images we will be rendering to. The "targets" are also known as render targets and include the attachments like colorattachment and depthattachment that describe where the rendering output will go to.
		 * 
		 * @since Karma 1.0.0
		 */
		void CreateRenderPass();

		/**
		 * @brief Framebuffers are collections of specific memory attachments that a render pass instance uses. They represent the actual memory that the rendering operations will write to.
		 *
		 * @since Karma 1.0.0
		 */
		void CreateFrameBuffers();

		/**
		 * @brief Command pools are opaque objects that command buffer memory is allocated from, and which allow the implementation to amortize the cost of resource creation.
		 *
		 * @since Karma 1.0.0
		 */
		void CreateCommandPool();

		/**
		 * @brief Creates the depth resources (image, imageview, and memory) for depth buffering in 3D rendering.
		 * 
		 * @note Depth buffering is a technique used in 3D computer graphics to determine the visibility of objects in a scene based on their distance from the viewer. It helps in rendering scenes with proper occlusion, ensuring that closer objects obscure those that are farther away.
		 * @since Karma 1.0.0
		 */
		void CreateDepthResources();

		/**
		 * @brief Finds a supported format from the list of candidate formats based on the desired tiling and features.
		 * Used in depth resource creation.
		 * 
		 * @see VulkanContext::FindDepthFormat()
		 * @since Karma 1.0.0
		 */
		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		
		/**
		 * @brief Finds a suitable depth format for depth buffering.
		 * 
		 * @see VulkanContext::CreateDepthResources()
		 * @since Karma 1.0.0
		 */
		VkFormat FindDepthFormat();

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
		 * @brief Copies data from a buffer to an image.
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
		 * @brief Recreates the swapchain, typically in response to window resizing or other events that invalidate the current swapchain.
		 * 
		 * This involves cleaning up the existing swapchain and its associated resources, and then creating a new swapchain with updated parameters.
		 * 
		 * @see VulkanRendererAPI::RecreateCommandBuffersAndSwapChain(), KarmaGuiRenderer::GiveLoopBeginControlToVulkan()
		 * @since Karma 1.0.0
		 */
		void RecreateSwapChain();

		/**
		 * @brief Cleans up the swapchain and its associated resources.
		 * 
		 * This involves destroying the framebuffers, image views, and the swapchain itself.
		 * 
		 * @see VulkanContext::RecreateSwapChain()
		 * @since Karma 1.0.0
		 */
		void CleanupSwapChain();

		/**
		 * @brief Enables or disables vertical synchronization (VSync) for the swapchain.
		 * 
		 * VSync synchronizes the frame rate of the application with the refresh rate of the monitor to prevent screen tearing.
		 * 
		 * @param bEnable						True to enable VSync, false to disable
		 * 
		 * @see WindowsWindow::SetVSync, VulkanContext::ChooseSwapPresentMode()
		 * @since Karma 1.0.0
		 */
		void SetVSync(bool bEnable);

		/**
		 * @brief Initializes the glslang library for shader compilation and processing.
		 * 
		 * @since Karma 1.0.0
		 */
		void Initializeglslang();
		
		/**
		 * @brief Registers a VulkanUniformBuffer for management by the VulkanContext.
		 * 
		 * @see VulkanContext::m_VulkanUBO
		 * @since Karma 1.0.0
		 */
		void RegisterUBO(const std::shared_ptr<VulkanUniformBuffer>& ubo);

		/**
		 * @brief Clears all registered VulkanUniformBuffers, freeing their resources.
		 * 
		 * @see VulkanContext::m_VulkanUBO
		 * @since Karma 1.0.0
		 */
		void ClearUBO();

		/**
		 * @brief Recreates all registered VulkanUniformBuffers, typically in response to swapchain recreation.
		 * 
		 * @see VulkanRendererAPI::RecreateCommandBuffersPipelineSwapchain()
		 * @since Karma 1.0.0
		 */
		void RecreateUBO();

		/**
		 * @brief Uploads data to the registered VulkanUniformBuffers for the specified frame index.
		 * 
		 * Typically called during rendering to update uniform buffer data for the current frame like so
		 * 
		 * @code{.cpp}
		 * vkCmdBeginRenderPass
		 *	vkCmdBindPipeline
		 *	vkCmdBindVertexBuffers
		 *	vkCmdBindIndexBuffer
		 *  UploadUBO()
		 *	vkCmdBindDescriptorSets // Descriptor sets include UBOs
		 *	vkCmdDrawIndexed
		 * vkCmdEndRenderPass
		 * @endcode
		 * 
		 * @param frameIndex						The index of the frame for which to upload UBO data
		 * 
		 * @see KarmaGuiRenderer::FrameRender, VulkanBuffer::UploadUniformBuffer
		 * @since Karma 1.0.0
		 */
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
		VkImageView GetDepthImageView() const { return m_DepthImageView; }
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
	};
}
