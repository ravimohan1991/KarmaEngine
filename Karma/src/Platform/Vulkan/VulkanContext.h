#pragma once

#define GLFW_INCLUDE_VULKAN
#include "Karma/Core.h"
#include "Karma/Renderer/GraphicsContext.h"
#include "GLFW/glfw3.h"
#include "vulkan/vulkan_core.h"
#include <optional>

namespace Karma
{
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

		void CreateInstance();

		// Validation layers
		bool CheckValidationLayerSupport();
		std::vector<const char*> GetRequiredExtensions();

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

		// Image views
		void CreateImageViews();

		// Graphics pipeline
		//void CreateGraphicsPipeline();

		//  Renderer pass
		void CreateRenderPass();

		// Framebuffers
		void CreateFrameBuffers();

		// Commandpool
		void CreateCommandPool();
		void CreateCommandBuffers();

		void CreateSemaphores();

		// Getters
		VkDevice* GetLogicalDevice() const { &m_device; }

		// Helper functions
		//static std::vector<char> ReadFile(const std::string& filename);
		//VkShaderModule CreateShaderModule(const std::vector<char>& code);

	private:
		GLFWwindow* m_windowHandle;
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;

		static bool bEnableValidationLayers;

		VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
		VkDevice m_device;
		VkQueue m_graphicsQueue;

		VkSurfaceKHR m_surface;
		VkQueue m_presentQueue;

		VkSwapchainKHR m_swapChain;
		std::vector<VkImage> m_swapChainImages;
		VkFormat m_swapChainImageFormat;
		VkExtent2D m_swapChainExtent;

		std::vector<VkImageView> m_swapChainImageViews;

		VkRenderPass m_renderPass;
		//VkPipelineLayout m_pipelineLayout;
		//VkPipeline m_graphicsPipeline;

		std::vector<VkFramebuffer> m_swapChainFrameBuffers;

		VkCommandPool m_commandPool;
		std::vector<VkCommandBuffer> m_commandBuffers;

		VkSemaphore m_imageAvailableSemaphore;
		VkSemaphore m_renderFinishedSemaphore;
	};
}