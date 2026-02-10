/**
 * @file VulkanDescriptorSets.h
 * @brief Header file for Vulkan descriptor sets management in a Vulkan rendering context.
 * @author Ravi Mohan (the_cowboy)
 * @date January 30, 2026
 * 
 * @copyright Karma Engine copyright(c) People of India
 */

#pragma once

#include "KarmaTypes.h"
#include <vulkan/vulkan.h>

namespace Karma
{
	class FVulkanDevice;
	
	class FVulkanDescriptorSetsLayoutInfo
	{
	public:
	   /**
		* @brief Constructor
		*
		* Adds all the Vulkan descriptor types currently needed.
		*
		* @since Karma 1.0.0
		*/
		FVulkanDescriptorSetsLayoutInfo()
		{
		}

		void SetLayoutTypes(const KarmaVector<VkDescriptorType>& NeededDescriptorTypes)
		{
			for (const VkDescriptorType& type : NeededDescriptorTypes)
			{
				m_LayoutTypes.Add(type, 1);
			}
		}
		
		/**
		* @brief Returns the number of descriptors of a specific type used in the layout, across all descriptor sets
		*
		* @param Type						The Vulkan descriptor type to query.
		* @return The number of descriptors of the specified type used in the layout.
		*
		* @since Karma 1.0.0
		*/
		inline uint32_t GetTypesUsed(VkDescriptorType Type) const
		{
			if (m_LayoutTypes.Contains(Type))
			{
				return m_LayoutTypes[Type];
			}
			else
			{
				return 0;
			}
		}
		
		/**
		 * @brief Structure representing a descriptor set layout.
		 */
		struct FSetLayout
		{
			/**
			 * @brief Layout bindings for this descriptor set, representing the individual
			 * descriptors and their configurations within the set.
			 */
			KarmaVector<VkDescriptorSetLayoutBinding> m_LayoutBindings;
			uint32_t m_Hash = 0;

			/**
			 * @brief The number of descriptor sets that use this layout. This is used for calculating the total number of descriptors
			 * needed for the pool allocation and appropriating the number of descriptor sets to allocate (vkAllocateDescriptorSets), from
			 * the pool, using this layout.
			 */
			uint32_t m_NumberOfDescriptorSets = 0;
			
			inline void GenerateHash()
			{
				// Simple hash generation based on bindings
				m_Hash = 0;
				for (const auto& binding : m_LayoutBindings)
				{
					m_Hash ^= std::hash<uint32_t>()(binding.binding) ^
					std::hash<uint32_t>()(binding.descriptorType) ^
					std::hash<uint32_t>()(binding.descriptorCount) ^
					std::hash<uint32_t>()(binding.stageFlags);
				}
			}
			
			friend uint32_t GetTypeHash(const FSetLayout& Layout)
			{
				return Layout.m_Hash;
			}
			
		};
		
		inline const KarmaVector<FSetLayout>& GetLayouts() const
		{
			return m_SetLayouts;
		}
		
		inline const KarmaMap<VkDescriptorType, uint32_t>& GetLayoutTypes() const
		{
			return m_LayoutTypes;
		}
		
	/*protected:*/
		void AddDescriptor(int32_t DescriptorSetIndex, const VkDescriptorSetLayoutBinding& Descriptor);
		void AddDescriptorSet(FSetLayout SetLayout);

	protected:
		/**
		 * @brief Map storing the count of each Vulkan descriptor type used in the layout.
		 */
		KarmaMap<VkDescriptorType, uint32_t> m_LayoutTypes;
		KarmaVector<FSetLayout> m_SetLayouts;
		
		uint32_t m_Hash = 0;
		VkPipelineBindPoint m_BindPoint = VK_PIPELINE_BIND_POINT_MAX_ENUM;
	};

	class FVulkanDescriptorSetsLayout : public FVulkanDescriptorSetsLayoutInfo
	{
	public:
		FVulkanDescriptorSetsLayout(FVulkanDevice* InDevice);
		~FVulkanDescriptorSetsLayout();

		void Compile();

		inline const KarmaVector<VkDescriptorSetLayout>& GetHandles() const
		{
			return m_LayoutHandles;
		}

		inline const KarmaVector<uint32_t>& GetHandleIds() const
		{
			return m_LayoutHandleIds;
		}

		inline const VkDescriptorSetAllocateInfo& GetAllocateInfo() const
		{
			return m_DescriptorSetsAllocateInfo;
		}

		inline uint32_t GetHash() const
		{
			return m_Hash;
		}

	private:
		FVulkanDevice* m_Device;
		KarmaVector<VkDescriptorSetLayout> m_LayoutHandles;
		KarmaVector<uint32_t> m_LayoutHandleIds;
		VkDescriptorSetAllocateInfo m_DescriptorSetsAllocateInfo;
	};

	struct FVulkanDescriptorSets
	{
		FVulkanDescriptorSets(const FVulkanDescriptorSetsLayout& InLayout);

		KarmaVector<KarmaVector<VkDescriptorSet>> m_DescriptorSets;
	};

	class FVulkanDescriptorPool
	{
	public:
		FVulkanDescriptorPool(FVulkanDevice* InDevice, const FVulkanDescriptorSetsLayout& InLayout/*, uint32_t MaxSetsAllocations*/);
		~FVulkanDescriptorPool();

		/**
		 * @brief Retrieves the Vulkan descriptor pool handle.
		 * 
		 * @return VkDescriptorPool The Vulkan descriptor pool handle.
		 * @since Karma 1.0.0
		 */
		inline VkDescriptorPool GetHandle() const
		{
			return m_DescriptorPool;
		}

		
		/**
		 * @brief Allocates descriptor sets from the pool based on the provided allocation info.
		 * 
		 * @param InDescriptorSetAllocateInfo				Information about the descriptor set allocation
		 * @param OutSets									Pointer to an array where allocated descriptor sets will be stored
		 * @return VkDescriptorSet							The first allocated descriptor set handle
		 * @since Karma 1.0.0
		 */
		void AllocateDescriptorSets(const FVulkanDescriptorSetsLayout& InLayout, FVulkanDescriptorSets& InDSets);

	private:
		FVulkanDevice* m_Device;
		uint32_t m_MaxDescriptorSets;
		const FVulkanDescriptorSetsLayout& m_Layout;
		VkDescriptorPool m_DescriptorPool;
	};

	class FVulkanDescriptorPoolsManager
	{
	private:
		FVulkanDevice* m_Device = nullptr;

	};
}
