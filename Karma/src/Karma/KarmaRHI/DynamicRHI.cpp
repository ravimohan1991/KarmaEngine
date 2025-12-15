#include "DynamicRHI.h"
#include "VulkanDynamicRHI.h"

namespace Karma
{
	// Globals.
	FDynamicRHI* GDynamicRHI = nullptr;


	FDynamicRHI* FDynamicRHI::Create()
	{
		// if RHI is ERHIInterfaceType::Vulkan
		return dynamic_cast<FDynamicRHI*>(new FVulkanDynamicRHI());
	}

	void RHIInit()
	{
		if (!GDynamicRHI)
		{
			GDynamicRHI = PlatformCreateDynamicRHI();
		}
	}

	void RHIExit()
	{
		delete GDynamicRHI;
		GDynamicRHI = nullptr;
	}
}