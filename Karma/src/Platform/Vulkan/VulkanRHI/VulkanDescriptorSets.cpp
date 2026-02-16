#include "VulkanDescriptorSets.h"
#include "VulkanDevice.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"

namespace Karma
{
	void FVulkanDescriptorSetsLayoutInfo::AddDescriptor(int32_t DescriptorSetIndex, const VkDescriptorSetLayoutBinding& Descriptor)
	{
		if (m_LayoutTypes.Contains(Descriptor.descriptorType))
		{
			m_LayoutTypes[Descriptor.descriptorType]++;
		}
		else
		{
			m_LayoutTypes.Add(Descriptor.descriptorType, 1);
		}

		if (DescriptorSetIndex > m_SetLayouts.Num())
		{
			m_SetLayouts.Resize(DescriptorSetIndex + 1);
		}

		FSetLayout& layout = m_SetLayouts[DescriptorSetIndex];
		layout.m_LayoutBindings.Add(Descriptor);
	}

	void FVulkanDescriptorSetsLayoutInfo::AddDescriptorSet(FSetLayout SetLayout)
	{
		m_SetLayouts.Add(SetLayout);

		for (const auto& layoutBindings : SetLayout.m_LayoutBindings)
		{
			if (m_LayoutTypes.Contains(layoutBindings.descriptorType))
			{
				m_LayoutTypes[layoutBindings.descriptorType]++;
			}
			else
			{
				m_LayoutTypes.Add(layoutBindings.descriptorType, 1);
			}
		}
	}

	FVulkanDescriptorSetsLayout::FVulkanDescriptorSetsLayout(FVulkanDevice* InDevice) : m_Device(InDevice), 
		m_DescriptorSetsAllocateInfo({})
	{
	}

	FVulkanDescriptorSetsLayout::~FVulkanDescriptorSetsLayout()
	{
		for (const auto& handle : m_LayoutHandles)
		{
			vkDestroyDescriptorSetLayout(m_Device->GetLogicalDevice(), handle, nullptr);
		}

		m_LayoutHandles.Clear();
	}	

	void FVulkanDescriptorSetsLayout::Compile()
	{
		for (const auto& setLayout : m_SetLayouts)
		{
			VkDescriptorSetLayout handle = VK_NULL_HANDLE;
			
			VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = static_cast<uint32_t>(setLayout.m_LayoutBindings.Num());
			layoutInfo.pBindings = setLayout.m_LayoutBindings.GetData(); // <------ contains the information of how many descriptors of each type are used in the layout, and their shader stage flags

			VkResult result = vkCreateDescriptorSetLayout(m_Device->GetLogicalDevice(), &layoutInfo, nullptr, &handle);
			KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create descriptor set layout!");

			m_LayoutHandles.Add(handle);
		}
	}

	FVulkanDescriptorPool::FVulkanDescriptorPool(FVulkanDevice* InDevice, const FVulkanDescriptorSetsLayout& Layout/*, uint32_t MaxSetsAllocations*/)
		: m_Device(InDevice), m_Layout(Layout),
		m_MaxDescriptorSets(0), m_DescriptorPool(VK_NULL_HANDLE)
	{
		// Descriptor sets number required to allocate max number of descriptor sets layout
		//m_MaxDescriptorSets = MaxSetsAllocations * Layout.GetLayouts().Num();

		KarmaMap<VkDescriptorType, uint32_t> descriptorTypeCounts;

		for (const auto& layout : Layout.GetLayouts())
		{
			m_MaxDescriptorSets += layout.m_NumberOfDescriptorSets;

			for (const auto& layoutBindings : layout.m_LayoutBindings)
			{
				if (descriptorTypeCounts.Contains(layoutBindings.descriptorType))
				{
					descriptorTypeCounts[layoutBindings.descriptorType] += layoutBindings.descriptorCount * layout.m_NumberOfDescriptorSets;
				}
				else
				{
					descriptorTypeCounts.Add(layoutBindings.descriptorType, layoutBindings.descriptorCount * layout.m_NumberOfDescriptorSets);
				}
			}
		}

		KarmaVector<VkDescriptorPoolSize> types;

		for (const auto& layoutType : Layout.GetLayoutTypes())
		{
			VkDescriptorPoolSize poolSize{};
			poolSize.type = VkDescriptorType(layoutType.first);
			//poolSize.descriptorCount = Layout.GetTypesUsed(poolSize.type) * MaxSetsAllocations;// the number of descriptors of this type across all the descriptor sets
			poolSize.descriptorCount = descriptorTypeCounts[layoutType.first];// the number of descriptors of this type across all the descriptor sets
			types.Add(poolSize);
		}

		// Note: A descriptor set may have multiple types of descriptors.
		// For example, a descriptor set may have both uniform buffers and combined image samplers.

		VkDescriptorPoolCreateInfo PoolInfoP{};
		PoolInfoP.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		PoolInfoP.maxSets = m_MaxDescriptorSets;// Max number of descriptor sets that can be allocated from this pool (via vkAllocateDescriptorSets)
		PoolInfoP.poolSizeCount = static_cast<uint32_t>(types.Num());
		PoolInfoP.pPoolSizes = types.GetData();

		VkResult result = vkCreateDescriptorPool(m_Device->GetLogicalDevice(), &PoolInfoP, nullptr, &m_DescriptorPool);
		KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to create descriptor pool!");
	}

	FVulkanDescriptorPool::~FVulkanDescriptorPool()
	{
		if (m_DescriptorPool != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorPool(m_Device->GetLogicalDevice(), m_DescriptorPool, nullptr);
			m_DescriptorPool = VK_NULL_HANDLE;

			//KR_CORE_INFO("Destroying descriptorpool");
		}
	}

	void FVulkanDescriptorPool::AllocateDescriptorSets(const FVulkanDescriptorSetsLayout& InLayout, FVulkanDescriptorSets& InDSets)
	{
		uint32_t setIndex = 0;
		for (const auto& layout : InLayout.GetLayouts())
		{
			KarmaVector<VkDescriptorSet>& dSets = InDSets.m_DescriptorSets[setIndex];

			for (auto& descriptorSet : dSets)
			{
				VkDescriptorSetAllocateInfo allocInfo{};
				allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				allocInfo.descriptorPool = m_DescriptorPool;
				allocInfo.descriptorSetCount = 1;
				allocInfo.pSetLayouts = &InLayout.GetHandles()[setIndex];

				VkResult result = vkAllocateDescriptorSets(m_Device->GetLogicalDevice(), &allocInfo, &descriptorSet);
				KR_CORE_ASSERT(result == VK_SUCCESS, "Failed to allocate descriptor set!");
			}

			setIndex++;
		}
	}

	FVulkanDescriptorSets::FVulkanDescriptorSets(FVulkanDevice* InDevice, const FVulkanDescriptorSetsLayout& InLayout)
	{
		m_Device = InDevice;

		m_DescriptorSets.Resize(InLayout.GetLayouts().Num());

		uint32_t setIndex = 0;

		for (const auto& layout : InLayout.GetLayouts())
		{
			m_DescriptorSets[setIndex++].Resize(layout.m_NumberOfDescriptorSets);
		}
	}

	void FVulkanDescriptorSets::UpdateUniformBufferDescriptorSet(VulkanUniformBuffer* Uniform, uint32_t SetLayoutIndex,
		uint32_t DescriptorSetIndex, uint32_t FrameIndex)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = Uniform->GetUniformBuffers()[FrameIndex];
		bufferInfo.offset = 0;
		bufferInfo.range = Uniform->GetBufferSize();

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = m_DescriptorSets[SetLayoutIndex][DescriptorSetIndex];
		descriptorWrite.dstBinding = 0; // Assuming the uniform buffer is bound to binding 0 in the shader
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;
		vkUpdateDescriptorSets(m_Device->GetLogicalDevice(), 1, &descriptorWrite, 0, nullptr);
	}

	void FVulkanDescriptorSets::UpdateTextureDescriptorSet(VulkanTexture* Texture, uint32_t SetLayoutIndex, uint32_t DescriptorSetIndex)
	{
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = Texture->GetImageView();
		imageInfo.sampler = Texture->GetImageSampler();

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = m_DescriptorSets[SetLayoutIndex][DescriptorSetIndex];
		descriptorWrite.dstBinding = 1; // Assuming the texture is bound to binding 1 in the shader
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &imageInfo;
		vkUpdateDescriptorSets(m_Device->GetLogicalDevice(), 1, &descriptorWrite, 0, nullptr);
	}
}
