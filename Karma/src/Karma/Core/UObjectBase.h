// Copyright Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	UObjectBase.h: Base class for UObject, defines low level functionality
=============================================================================*/

#pragma once

#include "krpch.h"

//#include "UObjectGlobals.h"

namespace Karma
{
	enum EObjectFlags;

	/**
	 * Low level implementation of UObject, should not be used directly in game code
	 * Taken from UE's UObjectBaseUtility.h.
	 */
	class KARMA_API UObjectBase
	{
	private:

		/** Flags used to track and report various object states. This needs to be 8 byte aligned on 32-bit
			platforms to reduce memory waste */
		EObjectFlags					ObjectFlags;

		/** Index into GObjectArray...very private. */
		int32_t							InternalIndex;

	public:
		/**
		 * Marks the object for garbage collection
		 */
		void MarkAsGarbage();

		/**
		 * Unmarks this object as Garbage.
		 */
		void ClearGarbage();
	};
}