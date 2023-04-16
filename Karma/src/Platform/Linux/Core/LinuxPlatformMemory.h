// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "krpch.h"

#include "Core/TrueCore/GenericPlatformMemory.h"

namespace Karma
{
	/**
	 * Linux implementation of the memory OS functions
	 **/
	struct KARMA_API FLinuxPlatformMemory : public FGenericPlatformMemory
	{
	};

	typedef FLinuxPlatformMemory FPlatformMemory;
}
