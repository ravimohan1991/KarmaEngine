#include "LinuxDynamicRHI.h"
#include "VulkanDynamicRHI.h"

#include "Log.h"

namespace Karma
{
#ifdef KR_LINUX_PLATFORM
	FDynamicRHI* PlatformCreateDynamicRHI()
	{
		FDynamicRHI* DynamicRHI = FDynamicRHI::CreateRHI();
		
		if (DynamicRHI)
		{
            KR_CORE_INFO("Created DynamicRHI for Linux OS");
		}

		return DynamicRHI;
	}
#endif
}
