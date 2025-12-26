/**
 * @file VulkanSwapChain.h
 * @brief
 * @author Ravi Mohan (the_cowboy)
 * @version 1.0
 * @date 20 December, 2025
 * 
 * @copyright Karma Engine copyright(c) People of India
 */

#pragma once

#include "VulkanRHI/VulkanDevice.h"
#include <vector>

struct GLFWwindow;

namespace Karma
{
	struct FVulkanSwapChainRecreateInfo
	{
		VkSwapchainKHR SwapChain;
		VkSurfaceKHR Surface;
	};

	class FVulkanSwapChain
	{
	public:
		static FVulkanSwapChain* Create(FVulkanDevice* InDevice);

		/**
		 * @brief Destroys the swapchain appropriately
		 *
		 * @since Karma 1.0.0
		 */
		void Destroy(FVulkanSwapChainRecreateInfo* RecreateInfo);

		///////////////// Getters /////////////////
		inline VkSwapchainKHR GetSwapChainHandle() const { return m_SwapChain; }

		inline VkExtent2D GetSwapChainExtent() const { return m_SwapChainExtent; }
		inline uint32_t GetMaxFramesInFlight() const { return MAX_FRAMES_IN_FLIGHT; }
		
		inline VkFormat GetSwapChainImageFormat() const { return m_SwapChainImageFormat; }

	private:

		/**
		 * @brief Creates Vulkan swapchain based upon the surface format and present mode
		 * 
		 * @param InDevice						The FVulkanDevice containing the LogicalDevice and GPU
		 * @since Karma 1.0.0
		 */
		FVulkanSwapChain(FVulkanDevice* InDevice);

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
		 * @note If this combination is not found, returns the first available format.
		 * 
		 * @param availableFormats						The available surface formats (from QuerySwapChainSupport())
		 *
		 * @see VulkanContext::CreateSwapChain()
		 * @since Karma 1.0.0
		 */
		static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

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

	protected:

		const VkInstance m_Instance;
		FVulkanDevice* m_Device;

		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
		VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;

		std::vector<VkImage> m_SwapChainImages;

		VkFormat m_SwapChainImageFormat;
		VkExtent2D m_SwapChainExtent;

		VkSurfaceFormatKHR m_SurfaceFormat;
		VkPresentModeKHR m_PresentMode;

		GLFWwindow* m_WindowHandle;

		uint32_t m_InternalWidth = 0;
		uint32_t m_InternalHeight = 0;

		bool bInternalFullScreen = false;

		uint32_t m_CurrentImageIndex;
		uint32_t m_SemaphoreIndex;

		// Number of images (to work upon (CPU side) whilst an image is being rendered (GPU side processing)) + 1
		// Clearly, MAX_FRAMES_IN_FLIGHT shouldn't exceed m_SwapChainImages.size()
		const uint32_t MAX_FRAMES_IN_FLIGHT = 2;
	};
}