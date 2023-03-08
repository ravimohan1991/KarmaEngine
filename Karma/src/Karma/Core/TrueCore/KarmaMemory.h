// Copyright Epic Games, Inc. All Rights Reserved.

// Taken from Unreal Engine with a soft corner of mind

#pragma once

#include "krpch.h"

#ifdef KR_WINDOWS_PLATFORM
#include "Platform/Windows/Core/WindowsPlatformMemory.h"
#elif KR_MAC_PLATFORM
#include "Platform/Mac/Core/MacPlatformMemory.h"
#endif

#include "UObjectAllocator.h"

namespace Karma
{
	// 32-bit unsigned integer
	typedef unsigned int		uint32;

	enum
	{
		// Default allocator alignment. If the default is specified, the allocator applies to engine rules.
		// Blocks >= 16 bytes will be 16-byte-aligned, Blocks < 16 will be 8-byte aligned. If the allocator does
		// not support allocation alignment, the alignment will be ignored.
		DEFAULT_ALIGNMENT = 0,

		// Minimum allocator alignment
		MIN_ALIGNMENT = 8,
	};


	struct KARMA_API FMemory
	{
		/** Some allocators can be given hints to treat allocations differently depending on how the memory is used, it's lifetime etc. */
		enum AllocationHints
		{
			None = -1,
			Default,
			Temporary,
			SmallPool,

			Max
		};

		/** @name Memory functions (wrapper for FPlatformMemory) */

		static FORCEINLINE void* Memmove(void* Dest, const void* Src, SIZE_T Count)
		{
			return FPlatformMemory::Memmove(Dest, Src, Count);
		}

		static FORCEINLINE int32_t Memcmp(const void* Buf1, const void* Buf2, SIZE_T Count)
		{
			return FPlatformMemory::Memcmp(Buf1, Buf2, Count);
		}

		static FORCEINLINE void* Memset(void* Dest, uint8_t Char, SIZE_T Count)
		{
			return FPlatformMemory::Memset(Dest, Char, Count);
		}

		template< class T >
		static FORCEINLINE void Memset(T& Src, uint8_t ValueToSet)
		{
			KR_CORE_ASSERT(!TIsPointer<T>::Value, "For pointers use the three parameters function");
			Memset(&Src, ValueToSet, sizeof(T));
		}

		static FORCEINLINE void* Memzero(void* Dest, SIZE_T Count)
		{
			return FPlatformMemory::Memzero(Dest, Count);
		}

		template< class T >
		static FORCEINLINE void Memzero(T& Src)
		{
			KR_CORE_ASSERT(!TIsPointer<T>::Value, "For pointers use the two parameters function");
			Memzero(&Src, sizeof(T));
		}

		static FORCEINLINE void* Memcpy(void* Dest, const void* Src, SIZE_T Count)
		{
			return FPlatformMemory::Memcpy(Dest, Src, Count);
		}

		template< class T >
		static FORCEINLINE void Memcpy(T& Dest, const T& Src)
		{
			KR_CORE_ASSERT(!TIsPointer<T>::Value, "For pointers use the three parameters function");
			Memcpy(&Dest, &Src, sizeof(T));
		}

		static FORCEINLINE void* BigBlockMemcpy(void* Dest, const void* Src, SIZE_T Count)
		{
			return FPlatformMemory::BigBlockMemcpy(Dest, Src, Count);
		}

		static FORCEINLINE void* StreamingMemcpy(void* Dest, const void* Src, SIZE_T Count)
		{
			return FPlatformMemory::StreamingMemcpy(Dest, Src, Count);
		}

		static FORCEINLINE void* ParallelMemcpy(void* Dest, const void* Src, SIZE_T Count, EMemcpyCachePolicy Policy = EMemcpyCachePolicy::StoreCached)
		{
			return FPlatformMemory::ParallelMemcpy(Dest, Src, Count, Policy);
		}

		// NOT FUNCTIONAL YET
		static FORCEINLINE void Memswap(void* Ptr1, void* Ptr2, SIZE_T Size)
		{
			FPlatformMemory::Memswap(Ptr1, Ptr2, Size);
		}

		//
		// C style memory allocation stubs that fall back to C runtime
		//
		static FORCEINLINE void* SystemMalloc(SIZE_T Size)
		{
			/* TODO: Trace! */
			return ::malloc(Size);
		}

		static FORCEINLINE void SystemFree(void* Ptr)
		{
			/* TODO: Trace! */
			::free(Ptr);
		}

		//
		// C style memory allocation stubs. Not functional as intended
		//

		static void* Malloc(SIZE_T Count, uint32 Alignment = DEFAULT_ALIGNMENT);
		/*static void* Realloc(void* Original, SIZE_T Count, uint32 Alignment = DEFAULT_ALIGNMENT);
		static void Free(void* Original);
		static SIZE_T GetAllocSize(void* Original);*/

		static FORCEINLINE void* MallocZeroed(SIZE_T Count, uint32 Alignment = DEFAULT_ALIGNMENT)
		{
			void* Memory = Malloc(Count, Alignment);
			Memzero(Memory, Count);
			return Memory;
		}
	};
}
