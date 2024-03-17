// Copyright Epic Games, Inc. All Rights Reserved.
/**
 * @file LinuxPlatformMemory.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the class FLinuxPlatformMemory.
 * @version 1.0
 * @date April 17, 2023
 */
#pragma once

#include "krpch.h"

#include "Core/TrueCore/GenericPlatformMemory.h"

namespace Karma
{
	/**
	 * @brief Linux implementation of the memory OS functions
	 **/
	struct KARMA_API FLinuxPlatformMemory : public FGenericPlatformMemory
	{
	};

	typedef FLinuxPlatformMemory FPlatformMemory;
}
