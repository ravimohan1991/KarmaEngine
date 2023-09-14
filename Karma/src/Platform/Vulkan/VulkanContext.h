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
	class RendererAPI;
	//class VulkanRendererAPI;
	class VulkanVertexArray;
	struct VulkanUniformBuffer;

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool IsComplete()
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class KARMA_API VulkanContext : public GraphicsContext
	{
	public:
		VulkanContext(GLFWwindow* windowHandle);
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
		void CreateLogicalDevice();

		// Swapchain
		void CreateSwapChain();
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		// Image views
		void CreateImageViews();

		//  Renderer pass
		void CreateRenderPass();

		// Framebuffers
		void CreateFrameBuffers();

		// CommandPool
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
