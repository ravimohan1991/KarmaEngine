/**
 * @file VulkanDynamicRHI.h
 * @brief
 * @author Ravi Mohan (the_cowboy)
 * @version 1.0
 * @date December 15, 2025
 * 
 * @copyright Karma Engine copyright(c) People of India
 */

#pragma once

#include "DynamicRHI.h"
#include "VulkanRHI.h"

namespace Karma
{
	template<typename TRHI>
	FORCEINLINE TRHI* GetDynamicRHI()
	{
		return static_cast<TRHI*>(GDynamicRHI);
	}

	class FVulkanDynamicRHI : public IVulkanDynamicRHI
	{
	public:
		static FVulkanDynamicRHI& Get() { return *GetDynamicRHI<FVulkanDynamicRHI>(); }

		/** Initialization constructor. */
		FVulkanDynamicRHI();

		virtual bool Init() override { return true; }
		/**
		 * @brief Shuts down the RHI.
		 *
		 * Cleans up resources and states used by the RHI.
		 */
		virtual void Shutdown() override {}

		/**
		 * @brief Presents the rendered frame to the display.
		 * 
		 * @since Karma 1.0.0
		 */
		virtual void Present() override {}
	};
}