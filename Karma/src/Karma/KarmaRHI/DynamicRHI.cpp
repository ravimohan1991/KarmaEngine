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
			KR_CORE_INFO("Karma RHI initializing");

			GDynamicRHI = PlatformCreateDynamicRHI();
			if (GDynamicRHI)
			{
				GDynamicRHI->Init();
			}
		}
	}

	void RHIExit()
	{
		if (GDynamicRHI != nullptr)
		{
			KR_CORE_INFO("Shutting down the RHI");
			GDynamicRHI->Shutdown();

			delete GDynamicRHI;
			GDynamicRHI = nullptr;

			KR_CORE_INFO("Karma RHI exodus");
		}
	}
}