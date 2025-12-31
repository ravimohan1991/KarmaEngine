#include "VulkanRenderPass.h"

namespace Karma
{
	VkRenderPass CreateVulkanRenderPass(FVulkanDevice& InDevice, const FVulkanRenderTargetLayout& RTLayout)
	{
		VkRenderPass OutRenderpass;

		FVulkanRenderPassBuilder<FVulkanSubpassDescription<VkSubpassDescription>, FVulkanSubpassDependency<VkSubpassDependency>, FVulkanAttachmentReference<VkAttachmentReference>, FVulkanAttachmentDescription<VkAttachmentDescription>, FVulkanRenderPassCreateInfo<VkRenderPassCreateInfo>> Creator(InDevice);
		OutRenderpass = Creator.Create(RTLayout);

		return OutRenderpass;
	}

	FVulkanRenderTargetLayout::FVulkanRenderTargetLayout(FVulkanRenderPassInfo& VRPInfo) : m_NumAttachmentDescriptions(0), m_NumColorAttachments(0)
	{
		// Color + depth attachment descriptions
		for (const auto& AttachmentInfo : VRPInfo.m_AttachmentsInfo)
		{
			VkAttachmentDescription& CurrentDescription = m_AttachmentDescriptions[m_NumAttachmentDescriptions++];

			CurrentDescription.flags = AttachmentInfo.AttachmentFlags;
			CurrentDescription.format = AttachmentInfo.AttachmentFormat;
			CurrentDescription.samples = AttachmentInfo.AttachmentSampleCount;
			CurrentDescription.loadOp = AttachmentInfo.AttachmentLoadOperation;
			CurrentDescription.storeOp = AttachmentInfo.AttachmentStoreOperation;
			CurrentDescription.stencilLoadOp = AttachmentInfo.AttachmentStencilLoadOperation;
			CurrentDescription.stencilStoreOp = AttachmentInfo.AttachmentStencilStoreOperation;
			CurrentDescription.initialLayout = AttachmentInfo.AttachmentInitialLayout;
			CurrentDescription.finalLayout = AttachmentInfo.AttachmentFinalLayout;
		}

		// Color attachment reference
		for (const auto& AttachmentRefInfo : VRPInfo.m_ColorAttachmentsRefInfo)
		{
			VkAttachmentReference& CurrentReference = m_ColorReferences[m_NumColorAttachments++];
			
			CurrentReference.attachment = AttachmentRefInfo.attachment;
			CurrentReference.layout = AttachmentRefInfo.layout;
		}

		// Depth attachment reference
		if (VRPInfo.bHasDepthAttachment)
		{
			bHasDepthStencil = true;

			m_DepthReference.attachment = VRPInfo.m_DepthAttachmentReference.attachment;
			m_DepthReference.layout = VRPInfo.m_DepthAttachmentReference.layout;
		}
		else
		{
			bHasDepthStencil = false;
		}

		m_RenderArea = VRPInfo.m_RenderArea;
	}

	FVulkanRenderPass::FVulkanRenderPass(FVulkanDevice& Device, const FVulkanRenderTargetLayout& RTLayout) : m_Device(Device)
	{
		m_RenderPass = CreateVulkanRenderPass(m_Device, RTLayout);
		m_Layout = RTLayout;
	}

	FVulkanRenderPass::~FVulkanRenderPass()
	{
		vkDestroyRenderPass(m_Device.GetLogicalDevice(), m_RenderPass, nullptr);
	}
}