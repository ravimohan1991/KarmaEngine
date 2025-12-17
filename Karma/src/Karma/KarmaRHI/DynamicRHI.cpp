#include "DynamicRHI.h"
#include "VulkanDynamicRHI.h"

namespace Karma
{
	// Globals.
	FDynamicRHI* GDynamicRHI = nullptr;
	ERHIInterfaceType GRHIInterfaceType = ERHIInterfaceType::Vulkan;

	FDynamicRHI* FDynamicRHI::CreateRHI()
	{
		// if RHI is ERHIInterfaceType::Vulkan
		if (GRHIInterfaceType == ERHIInterfaceType::Vulkan)
		{
			return static_cast<FDynamicRHI*>(new FVulkanDynamicRHI());
		}
		else
		{
			KR_CORE_WARN("RHI Interface Type not recognized!");
			return nullptr;
		}
	}

	void RHIInit()
	{
		if (!GDynamicRHI)
		{
			GDynamicRHI = PlatformCreateDynamicRHI();
			if (GDynamicRHI)
			{
				KR_CORE_INFO("############ Initialized DynamicRHI");
			}
		}
	}

	void RHIExit()
	{
		delete GDynamicRHI;
		GDynamicRHI = nullptr;

		KR_CORE_INFO("############ Shutdown DynamicRHI");
	}
}