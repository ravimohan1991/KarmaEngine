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

class FVulkanDescriptorSetsLayoutInfo
{
public:
	/**
	 * @brief Constructor
	 *
	 * Adds all the Vulkan descriptor types currently supported to the m_LayoutTypes map with initial count of 0.
	 *
	 * @since Karma 1.0.0
	 */
	FVulkanDescriptorSetsLayoutInfo()
	{
		for (uint32_t i = VK_DESCRIPTOR_TYPE_SAMPLER; i <= VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT; ++i)
		{
			m_LayoutTypes.Add(static_cast<VkDescriptorType>(i), 0);
		}

		m_LayoutTypes.Add(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
	}

	/**
	 * @brief Returns the number of descriptors of a specific type used in the layout.
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

	struct FSetLayout
	{
		KarmaVector<VkDescriptorSetLayoutBinding> m_LayoutBindings;
		uint32_t m_Hash;

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

private:
	KarmaMap<VkDescriptorType, uint32_t> m_LayoutTypes;

};