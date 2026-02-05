#include "VulkanDescriptorSets.h"
#include "VulkanDevice.h"

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
	}

	FVulkanDescriptorSetsLayout::FVulkanDescriptorSetsLayout(FVulkanDevice* InDevice) : m_Device(InDevice), 
		m_DescriptorSetsAllocateInfo({})
	{
	}

	FVulkanDescriptorSetsLayout::~FVulkanDescriptorSetsLayout()
	{
		// Hanldles are owned by FVulkanPipleLineStateCacheManager
		m_LayoutHandleIds.Clear();
	}	

	FVulkanDescriptorPool::FVulkanDescriptorPool(FVulkanDevice* InDevice, const FVulkanDescriptorSetsLayout& Layout, uint32_t MaxSetsAllocations)
		: m_Device(InDevice), m_MaxDescriptorSets(0), m_NumAllocatedDescriptorSets(0), m_PeakAllocatedDescriptorSets(0), m_Layout(Layout),
		m_DescriptorPool(VK_NULL_HANDLE)
	{
		// Descriptor sets number required to allocate max number of descriptor sets layout
		m_MaxDescriptorSets = MaxSetsAllocations * Layout.GetLayouts().Num();
		KarmaVector<VkDescriptorPoolSize> types;

		for (const auto& layoutType : Layout.GetLayoutTypes())
		{
			VkDescriptorPoolSize poolSize{};
			poolSize.type = VkDescriptorType(layoutType.first);
			poolSize.descriptorCount = Layout.GetTypesUsed(poolSize.type) * MaxSetsAllocations;
			types.Add(poolSize);
		}

		VkDescriptorPoolCreateInfo PoolInfoP{};
		PoolInfoP.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		PoolInfoP.maxSets = m_MaxDescriptorSets;
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

			KR_CORE_INFO("Destroying descriptorpool");
		}
	}

	VkDescriptorSet FVulkanDescriptorPool::AllocateDescriptorSet(const VkDescriptorSetAllocateInfo& InDescriptorSetAllocateInfo, VkDescriptorSet* OutSets)
	{
		VkDescriptorSetAllocateInfo AllocateInfo = InDescriptorSetAllocateInfo;
		AllocateInfo.descriptorPool = m_DescriptorPool;
		VkResult Result = vkAllocateDescriptorSets(m_Device->GetLogicalDevice(), &AllocateInfo, OutSets);
		if (Result != VK_SUCCESS)
		{
			KR_CORE_ERROR("Failed to allocate descriptor sets from the pool!");
			return VK_NULL_HANDLE;
		}
		m_NumAllocatedDescriptorSets += AllocateInfo.descriptorSetCount;
		if (m_NumAllocatedDescriptorSets > m_PeakAllocatedDescriptorSets)
		{
			m_PeakAllocatedDescriptorSets = m_NumAllocatedDescriptorSets;
		}
		return *OutSets;
	}
}