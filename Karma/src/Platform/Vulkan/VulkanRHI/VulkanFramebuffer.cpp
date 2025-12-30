#include "VulkanFramebuffer.h"

#include "VulkanDevice.h"
#include "VulkanRenderPass.h"

namespace Karma
{
	FVulkanFramebuffer::FVulkanFramebuffer(FVulkanDevice& Device, const FVulkanRenderTargetLayout& RTLayout, const FVulkanRenderPass& RenderPass)
	{
		VkFramebufferCreateInfo CreateInfo;
		CreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		CreateInfo.renderPass = RenderPass.GetHandle();
		CreateInfo.attachmentCount = m_ImageViews.Num();
		CreateInfo.pAttachments = m_ImageViews.GetData();
		//CreateInfo.width = 
	}
}