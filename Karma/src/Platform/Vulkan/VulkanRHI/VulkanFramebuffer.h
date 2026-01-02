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
	 * @brief May move to more abstract since any graphic api should use such render targets' information
	 * 
	 * @note UE uses FRHISetRenderTargetsInfo
	 */
	class FVulkanRenderTargetsInfo
	{
	public:
		struct ColorRenderTarget
		{
			VkImage m_ColorRenderTargetImages[MaxSimultaneousRenderTargets];
			VkImageView m_ColorRenderTargetViews[MaxSimultaneousRenderTargets];
		};

		struct DepthRenderTarget
		{
			VkImage m_DepthRenderTargetImage;
			VkImageView m_DepthRenderTargetView;
		};

		// Color rendertargets information
		ColorRenderTarget m_ColorRenderTargets;
		uint32_t m_NumColorRenderTargets;

		// Depth render target information
		bool bDepthRenderTarget = false;
		DepthRenderTarget m_DepthRenderTarget;
	};

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
		FVulkanFramebuffer(FVulkanDevice& Device, const FVulkanRenderTargetsInfo& InRTInfo,  const FVulkanRenderTargetLayout& RTLayout, const FVulkanRenderPass& RenderPass);

		VkFramebuffer GetHandle() const { return m_Framebuffer; }

	private:
		VkFramebuffer m_Framebuffer;
		VkRect2D m_RenderArea;

		KarmaVector<VkImageView> m_ImageViews;
	};
}