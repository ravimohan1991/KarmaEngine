/**
 * @file VulkanRHI.h
 * @brief
 * @author Ravi Mohan (the_cowboy)
 * @version 1.0
 * @date December 15, 2025
 * 
 * @copyright Karma Engine copyright(c) People of India
 */

#pragma once

#include "DynamicRHI.h"

namespace Karma
{
	struct IVulkanDynamicRHI : public FDynamicRHI
	{
		virtual ERHIInterfaceType GetInterfaceType() const override { return ERHIInterfaceType::Vulkan; }
	};

}