// Copyright Epic Games, Inc. All Rights Reserved.
/**
 * @file MacPlatformMemory.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the class FMacPlatformMemory.
 * @version 1.0
 * @date March 9, 2023
 */

#pragma once

#include "krpch.h"

#include "Core/TrueCore/GenericPlatformMemory.h"

namespace Karma
{
	/**
	 * @brief MacOS implementation of the memory OS functions
	 **/
	struct KARMA_API FMacPlatformMemory : public FGenericPlatformMemory
	{
	};

	typedef FMacPlatformMemory FPlatformMemory;
}
