#include "MacDynamicRHI.h"
#include "VulkanDynamicRHI.h"

#include "Log.h"

namespace Karma
{
#ifdef KR_MAC_PLATFORM
	FDynamicRHI* PlatformCreateDynamicRHI()
	{
		FDynamicRHI* DynamicRHI = FDynamicRHI::CreateRHI();
		
		if (DynamicRHI)
		{
			KR_CORE_INFO("Created DynamicRHI MacOS");
		}

		return DynamicRHI;
	}
#endif
}
