/**
 * @file VulkanRenderPass.h
 * @brief
 * @author Ravi Mohan (the_cowboy)
 * @version 1.0
 * @date December 21, 2025
 * 
 * @copyright Karma Engine copyright(c) People of India
 */

#pragma once

#include "VulkanRHI/VulkanDevice.h"
#include "Core/KarmaTypes.h"
#include "KarmaMemory.h"
#include "Core.h"

namespace Karma
{
	/**
	 * @brief Using the Curiously Recurring Template Pattern (CRTP) reversal?.
	 *
	 * In CRTP we are do static polymorphism i.e base class gains information about the derived class without virtual functions
	 */
	template <typename TAttachmentReferenceType>
	struct FVulkanAttachmentReference : public TAttachmentReferenceType
	{
		FVulkanAttachmentReference()
		{
			ZeroStruct();
		}

		FVulkanAttachmentReference(const VkAttachmentReference& AttachmentReferenceIn, VkImageAspectFlags AspectMask)
		{
			SetAttachment(AttachmentReferenceIn, AspectMask);
		}

		/**
		 * @brief Disable method enforced here (checkNoEntry())
		 * 
		 * This "poison" is to ensure correct use (or trap the misuse) at call site
		 * 
		 * ❌ Input attachments: mutation blocked (checkNoEntry())
		 * FVulkanAttachmentReference<VkInputAttachmentReference> inputRef;  
		 * inputRef.SetAttachment(...);  // Asserts/crashes - prevents bugs
		 *
		 * ✅ Color/depth attachments: mutation allowed
		 * FVulkanAttachmentReference<VkAttachmentReference> colorRef;  
		 * colorRef.SetAttachment(ref, mask);  // Copies attachment/layout safely
		 *
		 * This selectively enables mutation only for standard attachments
		 * 
		 * @since Karma 1.0.0
		 */
		inline void SetAttachment(const VkAttachmentReference& AttachmentReferenceIn, VkImageAspectFlags AspectMask) { checkNoEntry(); }
		inline void SetAttachment(const FVulkanAttachmentReference<TAttachmentReferenceType>& AttachmentReferenceIn, VkImageAspectFlags AspectMask) { *this = AttachmentReferenceIn; }
		inline void SetDepthStencilAttachment(const VkAttachmentReference& AttachmentReferenceIn, const VkAttachmentReferenceStencilLayout* StencilReference, VkImageAspectFlags AspectMask, bool bSupportsParallelRendering) { checkNoEntry(); }
		inline void ZeroStruct() {}
		inline void SetAspect(uint32_t Aspect) {}
	};

	/**
	 * @brief This is template specialization where we define or implement the generic SetAttachment for VkAttachmentReference
	 */
	template <>
	inline void FVulkanAttachmentReference<VkAttachmentReference>::SetAttachment(const VkAttachmentReference& AttachmentReferenceIn, VkImageAspectFlags AspectMask)
	{
		attachment = AttachmentReferenceIn.attachment;
		layout = AttachmentReferenceIn.layout;
	}

	template <>
	inline void FVulkanAttachmentReference<VkAttachmentReference>::SetDepthStencilAttachment(const VkAttachmentReference& AttachmentReferenceIn,
		const VkAttachmentReferenceStencilLayout* StencilReference, VkImageAspectFlags AspectMask, bool bSupportsParallelRendering)
	{
		attachment = AttachmentReferenceIn.attachment;
		const VkImageLayout StencilLayout = StencilReference ? StencilReference->stencilLayout : VK_IMAGE_LAYOUT_UNDEFINED;

		layout = AttachmentReferenceIn.layout; //GetMergedDepthStencilLayout(AttachmentReferenceIn.layout, StencilLayout);
	}

	/**
	 * @brief Template (for VkAttachmentDescription) definition of FVulkanAttachmentDescription
	 *
	 * Is this the CRTP reversal?
	 * 
	 * @since Karma 1.0.0
	 */
	template<typename TAttachmentDescriptionType>
	struct FVulkanAttachmentDescription
	{
	};

	/**
	 * @brief Specializing (?) FVulkanAttachmentDescription for VkAttachmentDescription
	 */
	template<>
	struct FVulkanAttachmentDescription<VkAttachmentDescription>
		: public VkAttachmentDescription
	{
		FVulkanAttachmentDescription()
		{
			FMemory::Memzero(this, sizeof(VkAttachmentDescription));
		}

		FVulkanAttachmentDescription(const VkAttachmentDescription& InDesc)
		{
			flags = InDesc.flags;
			format = InDesc.format;
			samples = InDesc.samples;
			loadOp = InDesc.loadOp;
			storeOp = InDesc.storeOp;
			stencilLoadOp = InDesc.stencilLoadOp;
			stencilStoreOp = InDesc.stencilStoreOp;
			initialLayout = InDesc.initialLayout;
			finalLayout = InDesc.finalLayout;
		}

		/**
		 * @brief Seems like we are not supporting VkAttachmentDescriptionStencilLayout
		 */
		FVulkanAttachmentDescription(const VkAttachmentDescription& InDesc, const VkAttachmentDescriptionStencilLayout* InStencilDesc, bool bSupportsParallelRendering)
		{
			flags = InDesc.flags;
			format = InDesc.format;
			samples = InDesc.samples;
			loadOp = InDesc.loadOp;
			storeOp = InDesc.storeOp;
			stencilLoadOp = InDesc.stencilLoadOp;
			stencilStoreOp = InDesc.stencilStoreOp;

			/*const bool bHasStencilLayout = VulkanFormatHasStencil(InDesc.format) && (InStencilDesc != nullptr);
			const VkImageLayout StencilInitialLayout = bHasStencilLayout ? InStencilDesc->stencilInitialLayout : VK_IMAGE_LAYOUT_UNDEFINED;
			initialLayout = GetMergedDepthStencilLayout(InDesc.initialLayout, StencilInitialLayout);
			const VkImageLayout StencilFinalLayout = bHasStencilLayout ? InStencilDesc->stencilFinalLayout : VK_IMAGE_LAYOUT_UNDEFINED;
			finalLayout = GetMergedDepthStencilLayout(InDesc.finalLayout, StencilFinalLayout);
			*/
		}
	};

	/**
	 * @brief Template (for VkSubpassDescription) definition of FVulkanSubpassDescription
	 * 
	 * @since Karma 1.0.0
	 */
	template <typename TSubpassDescriptionType>
	class FVulkanSubpassDescription
	{
	};

	/**
	 * @brief Specialized definition of FVulkanSubpassDescription 
	 */
	template<>
	struct FVulkanSubpassDescription<VkSubpassDescription>
		: public VkSubpassDescription
	{
		FVulkanSubpassDescription()
		{
			FMemory::Memzero(this, sizeof(VkSubpassDescription));
			pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		}

		void SetColorAttachments(const KarmaVector<FVulkanAttachmentReference<VkAttachmentReference>>& ColorAttachmentReferences, int OverrideCount = -1)
		{
			colorAttachmentCount = (OverrideCount == -1) ? ColorAttachmentReferences.Num() : OverrideCount;
			pColorAttachments = ColorAttachmentReferences.GetData();
		}

		/*void SetResolveAttachments(const KarmaVector<FVulkanAttachmentReference<VkAttachmentReference>>& ResolveAttachmentReferences)
		{
			if (ResolveAttachmentReferences.Num() > 0)
			{
				check(colorAttachmentCount == ResolveAttachmentReferences.Num());
				pResolveAttachments = ResolveAttachmentReferences.GetData();
			}
		}*/

		void SetDepthStencilAttachment(FVulkanAttachmentReference<VkAttachmentReference>* DepthStencilAttachmentReference)
		{
			pDepthStencilAttachment = static_cast<VkAttachmentReference*>(DepthStencilAttachmentReference);
		}

		/*void SetInputAttachments(FVulkanAttachmentReference<VkAttachmentReference>* InputAttachmentReferences, uint32 NumInputAttachmentReferences)
		{
			pInputAttachments = static_cast<VkAttachmentReference*>(InputAttachmentReferences);
			inputAttachmentCount = NumInputAttachmentReferences;
		}

		void SetShadingRateAttachment(void* /* ShadingRateAttachmentInfo)
		{
			// No-op without VK_KHR_create_renderpass2
		}

		void SetMultiViewMask(uint32_t Mask)
		{
			// No-op without VK_KHR_create_renderpass2
		}*/
	};

	/**
	 * @brief The number of render-targets that may be simultaneously written to.
	 * 
	 * @note May move to different header file with appropriate definitions
	 */
	enum
	{
		MaxSimultaneousRenderTargets = 8,
		MaxSimultaneousRenderTargets_NumBits = 3,
	};
    static_assert(MaxSimultaneousRenderTargets <= (1 << MaxSimultaneousRenderTargets_NumBits), "MaxSimultaneousRenderTargets will not fit on MaxSimultaneousRenderTargets_NumBits");

	class FVulkanRenderTargetLayout
	{
	public:
		//FVulkanRenderTargetLayout(const FGraphicsPipelineStateInitializer& Initializer);
		
		//FVulkanRenderTargetLayout(FVulkanDevice& InDevice, const FRHISetRenderTargetsInfo& RTInfo);
		//FVulkanRenderTargetLayout(FVulkanDevice& InDevice, const FRHIRenderPassInfo& RPInfo, VkImageLayout CurrentDepthLayout, VkImageLayout CurrentStencilLayout);

		inline const VkAttachmentReference* GetColorAttachmentReferences() const { return m_NumColorAttachments > 0 ? m_ColorReferences : nullptr; }
		inline const VkAttachmentReference* GetResolveAttachmentReferences() const { return bHasResolveAttachments ? m_ResolveReferences : nullptr; }
		inline const VkAttachmentReference* GetDepthAttachmentReference() const { return bHasDepthStencil ? &m_DepthReference : nullptr; }
		inline const VkAttachmentReferenceStencilLayout* GetStencilAttachmentReference() const { return bHasDepthStencil ? &m_StencilReference : nullptr; }
		//inline const VkAttachmentReference* GetFragmentDensityAttachmentReference() const { return bHasFragmentDensityAttachment ? &FragmentDensityReference : nullptr; }

		inline uint32_t GetNumColorAttachments() const { return m_NumColorAttachments; }
		inline uint32_t GetNumAttachmentDescriptions() const { return m_NumAttachmentDescriptions; }

		inline const VkAttachmentDescription* GetAttachmentDescriptions() const { return m_AttachmentDescriptions; }
		//inline const VkAttachmentDescriptionStencilLayout* GetStencilDescription() const { return bHasDepthStencil ? &m_StencilDescription : nullptr; }

	private:
		VkAttachmentReference m_ColorReferences[MaxSimultaneousRenderTargets];
		
		// Probably not required because of we are using default MSAA?
		VkAttachmentReference m_ResolveReferences[MaxSimultaneousRenderTargets];
		
		// Depth attachment ref
		VkAttachmentReference m_DepthReference;
		VkAttachmentReferenceStencilLayout m_StencilReference;

		uint8_t m_NumColorAttachments;
		uint8_t m_NumAttachmentDescriptions;

		VkAttachmentDescription m_AttachmentDescriptions[MaxSimultaneousRenderTargets * 2 + 2];
		//VkAttachmentDescriptionStencilLayout m_StencilDescription;

		uint8_t bHasResolveAttachments;
		
		// Do we have a depth stencil
		uint8_t bHasDepthStencil;
	};

	class FVulkanRenderPass
	{
	public:
		inline VkRenderPass GetHandle() const { return m_RenderPass; }

	private:
		VkRenderPass m_RenderPass;
	};

	template <typename TSubpassDescriptionClass, typename TSubpassDependencyClass, typename TAttachmentReferenceClass, typename TAttachmentDescriptionClass, typename TRenderPassCreateInfoClass>
	class FVulkanRenderPassBuilder
	{
	public:
		FVulkanRenderPassBuilder(FVulkanDevice& InDevice)
			: m_Device(InDevice)
			, m_CorrelationMask(0)
		{
		}

		void BuildCreateInfo(const FVulkanRenderTargetLayout& RTLayout)
		{
			uint32_t NumSubpasses = 0;
			uint32_t NumDependencies = 0;

			// Grab (and optionally convert) attachment references.
			uint32_t NumColorAttachments = RTLayout.GetNumColorAttachments();

			// Do we have a depth attachment
			const bool bHasDepthStencilAttachmentReference = (RTLayout.GetDepthAttachmentReference() != nullptr);
			
			for (uint32_t ColorAttachment = 0; ColorAttachment < NumColorAttachments; ++ColorAttachment)
			{
				m_ColorAttachmentReferences.Add(TAttachmentReferenceClass(RTLayout.GetColorAttachmentReferences()[ColorAttachment], 0));
				
				// For MSAA
				/*if (RTLayout.GetResolveAttachmentReferences() != nullptr)
				{
					m_ResolveAttachmentReferences.Add(TAttachmentReferenceClass(RTLayout.GetResolveAttachmentReferences()[ColorAttachment], 0));
				}*/
			}

			uint32_t DepthInputAttachment = VK_ATTACHMENT_UNUSED;
			VkImageLayout DepthInputAttachmentLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			VkImageAspectFlags DepthInputAspectMask = 0;

			if (bHasDepthStencilAttachmentReference)
			{
				// For depth attachment reference, attachment and layout should also be given by GetDepthAttachmentReference only
				m_DepthStencilAttachmentReference.SetDepthStencilAttachment(*RTLayout.GetDepthAttachmentReference(), RTLayout.GetStencilAttachmentReference(), 0, false/*m_Device.SupportsParallelRendering()*/);
				
				// Why are these needed when attachment and layout are assigned in m_DepthStencilAttachmentReference
				m_DepthStencilAttachment.attachment = RTLayout.GetDepthAttachmentReference()->attachment;
				m_DepthStencilAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			}

			// main subpass
			{
				TSubpassDescriptionClass& SubpassDesc = m_SubpassDescriptions[NumSubpasses++];

				SubpassDesc.SetColorAttachments(m_ColorAttachmentReferences, NumColorAttachments);

				if (bHasDepthStencilAttachmentReference)
				{
					SubpassDesc.SetDepthStencilAttachment(&m_DepthStencilAttachmentReference);
				}
			}

			for (uint32_t Attachment = 0; Attachment < RTLayout.GetNumAttachmentDescriptions(); ++Attachment)
			{
				if (bHasDepthStencilAttachmentReference && (Attachment == m_DepthStencilAttachmentReference.attachment))
				{
					//m_AttachmentDescriptions.Add(TAttachmentDescriptionClass(RTLayout.GetAttachmentDescriptions()[Attachment], RTLayout.GetStencilDescription(), false/*Device.SupportsParallelRendering()*/));
				}
				else
				{
					m_AttachmentDescriptions.Add(TAttachmentDescriptionClass(RTLayout.GetAttachmentDescriptions()[Attachment]));
				}
			}

			/*m_CreateInfo.attachmentCount = m_AttachmentDescriptions.Num();
			m_CreateInfo.pAttachments = m_AttachmentDescriptions.GetData();
			m_CreateInfo.subpassCount = NumSubpasses;
			m_CreateInfo.pSubpasses = m_SubpassDescriptions;
			m_CreateInfo.dependencyCount = NumDependencies;
			m_CreateInfo.pDependencies = m_SubpassDependencies;*/
		}

	private:
		TSubpassDescriptionClass m_SubpassDescriptions[8];
		TSubpassDependencyClass m_SubpassDependencies[8];

		KarmaVector<TAttachmentReferenceClass> m_ColorAttachmentReferences;
		KarmaVector<TAttachmentReferenceClass> m_ResolveAttachmentReferences;

		KarmaVector<TAttachmentDescriptionClass> m_AttachmentDescriptions;

		TAttachmentReferenceClass m_DepthStencilAttachmentReference;
		TAttachmentReferenceClass m_DepthStencilAttachment;

		TRenderPassCreateInfoClass m_CreateInfo;
		FVulkanDevice& m_Device;

		uint32_t m_CorrelationMask;
	};
}