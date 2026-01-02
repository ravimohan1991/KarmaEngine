#include "VulkanFramebuffer.h"

#include "VulkanDevice.h"
#include "VulkanRenderPass.h"

namespace Karma
{
	FVulkanFramebuffer::FVulkanFramebuffer(FVulkanDevice& Device, const FVulkanRenderTargetsInfo& InRTInfo, const FVulkanRenderTargetLayout& RTLayout, const FVulkanRenderPass& RenderPass)
	{
		for (const auto& renderTarget : InRTInfo.m_ColorRenderTargets.m_ColorRenderTargetViews)
		{
			m_ImageViews.Add(renderTarget);
		}

		if (InRTInfo.bDepthRenderTarget)
		{
			m_ImageViews.Add(InRTInfo.m_DepthRenderTarget.m_DepthRenderTargetView);
		}

		VkFramebufferCreateInfo createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.renderPass = RenderPass.GetHandle();
		createInfo.attachmentCount = m_ImageViews.Num();
		createInfo.pAttachments = m_ImageViews.GetData();
		createInfo.width = RTLayout.GetRenderArea().extent.width;
		createInfo.height = RTLayout.GetRenderArea().extent.height;
		createInfo.layers = 1;

		VkResult result = vkCreateFramebuffer(Device.GetLogicalDevice(), &createInfo, nullptr, &m_Framebuffer);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create frame buffer");

		m_RenderArea = RTLayout.GetRenderArea();
	}
}