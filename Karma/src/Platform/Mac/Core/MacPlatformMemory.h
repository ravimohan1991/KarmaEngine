// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "krpch.h"

#include "Core/TrueCore/GenericPlatformMemory.h"

namespace Karma
{
	/**
	 * Windows implementation of the memory OS functions
	 **/
	struct KARMA_API FMacPlatformMemory : public FGenericPlatformMemory
	{
	};

	typedef FMacPlatformMemory FPlatformMemory;
}
