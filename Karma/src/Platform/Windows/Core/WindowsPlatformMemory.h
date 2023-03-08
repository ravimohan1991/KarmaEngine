// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "krpch.h"

#include "Core/TrueCore/GenericPlatformMemory.h"

namespace Karma
{
	/**
	 * Windows implementation of the memory OS functions
	 **/
	struct KARMA_API FWindowsPlatformMemory : public FGenericPlatformMemory
	{
		enum EMemoryCounterRegion
		{
			MCR_Invalid, // not memory
			MCR_Physical, // main system memory
			MCR_GPU, // memory directly a GPU (graphics card, etc)
			MCR_GPUSystem, // system memory directly accessible by a GPU
			MCR_TexturePool, // presized texture pools
			MCR_StreamingPool, // amount of texture pool available for streaming.
			MCR_UsedStreamingPool, // amount of texture pool used for streaming.
			MCR_GPUDefragPool, // presized pool of memory that can be defragmented.
			MCR_SamplePlatformSpecifcMemoryRegion,
			MCR_PhysicalLLM, // total physical memory displayed in the LLM stats (on consoles CPU + GPU)
			MCR_MAX
		};
	};

	typedef FWindowsPlatformMemory FPlatformMemory;
}