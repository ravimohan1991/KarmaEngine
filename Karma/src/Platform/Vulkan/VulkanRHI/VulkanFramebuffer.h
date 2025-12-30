/**
 * @file VulkanFramebuffer.h
 * @brief Contains the Vulkan framebuffer class to hold pixel color/depth data
 * @author Ravi Mohan (the_cowboy)
 * @version 1.0
 * @date December 30, 2025
 * 
 * @copyright Karma Engine copyright(c) People of India
 */

#pragma once

#include <vulkan/vulkan_core.h>

namespace Karma
{
	// Forward declaration
	class FVulkanDevice;
	class FVulkanRenderTargetLayout;

	class FVulkanRenderPass;

	/**
	 * @brief Actual Vulkan framebuffer class
	 * 
	 * Framebuffers consists of rendertargets which are filled with per pixel information (color/depth).
	 * Specifically framebuffer consists of VkImages with appropriate VkImageViews which receive fragment shader output
	 * data. In terms of graphics pipeline stages, this output is stored in framebuffer imageviews after fragment shader
	 * stage and before color blending stage.
	 */
	class FVulkanFramebuffer
	{
	public:
		FVulkanFramebuffer(FVulkanDevice& Device, const FVulkanRenderTargetLayout& RTLayout, const FVulkanRenderPass& RenderPass);

	private:
		VkFramebuffer m_FrameBuffer;
		VkRect2D m_RenderArea;

		KarmaVector<VkImageView> m_ImageViews;
	};
}