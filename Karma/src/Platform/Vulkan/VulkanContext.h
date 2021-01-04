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

		bool IsComplete()
		{
			return graphicsFamily.has_value();
		}
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

		// Physical device
		void PickPhysicalDevice();
		bool IsDeviceSuitable(VkPhysicalDevice device);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

		// Logical device
		void CreateLogicalDevice();

	private:
		GLFWwindow* m_windowHandle;
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;

		static bool bEnableValidationLayers;

		VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
		VkDevice m_device;
		VkQueue m_graphicsQueue;
	};
}