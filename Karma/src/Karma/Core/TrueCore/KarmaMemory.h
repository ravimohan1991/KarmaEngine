/**
 * @file KarmaMemory.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the class FMemory..
 * @version 1.0
 * @date March 8, 2023
 *
 * @copyright Copyright Epic Games, Inc. All Rights Reserved.
 */

// Taken from Unreal Engine with a soft corner of mind

#pragma once

#include "krpch.h"

#ifdef KR_WINDOWS_PLATFORM
#include "Platform/Windows/Core/WindowsPlatformMemory.h"
#elif KR_MAC_PLATFORM
#include "Platform/Mac/Core/MacPlatformMemory.h"
#elif KR_LINUX_PLATFORM
#include "Platform/Linux/Core/LinuxPlatformMemory.h"
#endif

#include "UObjectAllocator.h"

namespace Karma
{
	/**
	 * @brief 32-bit unsigned integer
	 */
	typedef unsigned int		uint32;

	/**
	 * @brief Alignment enum
	 */
	enum
	{
		DEFAULT_ALIGNMENT = 0,///<Default allocator alignment. If the default is specified, the allocator applies to engine rules. Blocks >= 16 bytes will be 16-byte-aligned, Blocks < 16 will be 8-byte aligned. If the allocator does not support allocation alignment, the alignment will be ignored.

		MIN_ALIGNMENT = 8,///<Minimum allocator alignment
	};

	/**
	 * @brief Class with memory relevant functions
	 */
	struct KARMA_API FMemory
	{
		/**
		 * @brief Some allocators can be given hints to treat allocations differently depending on how the memory is used, it's lifetime etc.
		 */
		enum AllocationHints
		{
			None = -1,
			Default,
			Temporary,
			SmallPool,

			Max
		};

		// @name Memory functions (wrapper for FPlatformMemory)

		/**
		 * @brief Copies count bytes of characters from Src to Dest. If some regions of the source
		 * area and the destination overlap, memmove ensures that the original source bytes
		 * in the overlapping region are copied before being overwritten.
		 *
		 * @remark make sure that the destination buffer is the same size or larger than the source buffer!
		 *
		 * @param Dest					Where to copy bytes to
		 * @param Src					Where to copy bytes from
		 *
		 * @param Count					Size of bytes to be copied
		 *
		 * @since Karma 1.0.0
		 */
		static FORCEINLINE void* Memmove(void* Dest, const void* Src, SIZE_T Count)
		{
			return FPlatformMemory::Memmove(Dest, Src, Count);
		}

		/**
		 * @brief Compares first Count bytes of memory of Buf1 and Buf2
		 *
		 * @param Buf1				A block of memory
		 * @param Buf2				A block of memory
		 *
		 * @param Count				Number of bytes to be compared
		 *
		 * @since Karma 1.0.0
		 */
		static FORCEINLINE int32_t Memcmp(const void* Buf1, const void* Buf2, SIZE_T Count)
		{
			return FPlatformMemory::Memcmp(Buf1, Buf2, Count);
		}

		/**
		 * @brief Copy value Char in each of first Count characters of object pointed to by Dest
		 *
		 * @param Dest				Copy the value to
		 * @param Char				Value to be copied
		 *
		 * @param Count				Number of characters in the object to be copied to
		 *
		 * @since Karma 1.0.0
		 */
		static FORCEINLINE void* Memset(void* Dest, uint8_t Char, SIZE_T Count)
		{
			return FPlatformMemory::Memset(Dest, Char, Count);
		}

		/**
		 * @brief Copy value ValueToSet to template Src
		 *
		 * @param Src						Copy the value to
		 * @param ValueToSet				Value to be copied
		 *
		 * @since Karma 1.0.0
		 */
		template< class T >
		static FORCEINLINE void Memset(T& Src, uint8_t ValueToSet)
		{
			KR_CORE_ASSERT(!TIsPointer<T>::Value, "For pointers use the three parameters function");
			Memset(&Src, ValueToSet, sizeof(T));
		}

		/**
		 * @brief Zeros the Count number of characters of object pointed by Dest
		 *
		 * @param Dest				Set zeros of the object
		 * @param Count				Number of characters
		 *
		 * @since Karma 1.0.0
		 */
		static FORCEINLINE void* Memzero(void* Dest, SIZE_T Count)
		{
			return FPlatformMemory::Memzero(Dest, Count);
		}

		/**
		 * @brief Zeros the template Src
		 *
		 * @param Src				The reference to the variable to be zeroed
		 *
		 * @since Karma 1.0.0
		 */
		template< class T >
		static FORCEINLINE void Memzero(T& Src)
		{
			KR_CORE_ASSERT(!TIsPointer<T>::Value, "For pointers use the two parameters function");
			Memzero(&Src, sizeof(T));
		}

		/**
		 * @brief Copies Count bytes from the object pointed by Src to the object pointed by Dest
		 *
		 * @param Src				Object to copy from
		 * @param Dest				Object to copy to
		 *
		 * @param Count				Number of bytes to be copied
		 *
		 * @remark If both objects overlap, behavior is undefined
		 *
		 * @since Karma 1.0.0
		 */
		static FORCEINLINE void* Memcpy(void* Dest, const void* Src, SIZE_T Count)
		{
			return FPlatformMemory::Memcpy(Dest, Src, Count);
		}

		/**
		 * @brief Copies  from the object pointed by Src to the object pointed by Dest
		 *
		 * @param Src				Object to copy from
		 * @param Dest				Object to copy to
		 *
		 * @since Karma 1.0.0
		 */
		template< class T >
		static FORCEINLINE void Memcpy(T& Dest, const T& Src)
		{
			KR_CORE_ASSERT(!TIsPointer<T>::Value, "For pointers use the three parameters function");
			Memcpy(&Dest, &Src, sizeof(T));
		}

		/**
		 * @brief Memcpy optimized for big blocks.
		 *
		 * @param Dest				Object to copy to
		 * @param Src				Object to copy from
		 *
		 * @param Count				Number of bytes to be copied
		 *
		 * @see FGenericPlatformMemory::Memcpy
		 *
		 * @since Karma 1.0.0
		 */
		static FORCEINLINE void* BigBlockMemcpy(void* Dest, const void* Src, SIZE_T Count)
		{
			return FPlatformMemory::BigBlockMemcpy(Dest, Src, Count);
		}

		/**
		 * @brief On some platforms memcpy optimized for big blocks that avoid L2 cache pollution are available
		 *
		 * @param Dest				Object to copy to
		 * @param Src				Object to copy from
		 *
		 * @param Count				Number of bytes to be copied
		 *
		 * @see FGenericPlatformMemory::Memcpy
		 *
		 * @since Karma 1.0.0
		 */
		static FORCEINLINE void* StreamingMemcpy(void* Dest, const void* Src, SIZE_T Count)
		{
			return FPlatformMemory::StreamingMemcpy(Dest, Src, Count);
		}

		/**
		 * @brief On some platforms memcpy can be distributed over multiple threads for throughput.
		 *
		 * @param Dest				Object to copy to
		 * @param Src				Object to copy from
		 *
		 * @param Policy				Is copy result immidiately or gaply accessed by CPU
		 *
		 * @since Karma 1.0.0
		 */
		static FORCEINLINE void* ParallelMemcpy(void* Dest, const void* Src, SIZE_T Count, EMemcpyCachePolicy Policy = EMemcpyCachePolicy::StoreCached)
		{
			return FPlatformMemory::ParallelMemcpy(Dest, Src, Count, Policy);
		}

		// NOT FUNCTIONAL YET
		static FORCEINLINE void Memswap(void* Ptr1, void* Ptr2, SIZE_T Size)
		{
			FPlatformMemory::Memswap(Ptr1, Ptr2, Size);
		}

		/**
		 * @brief C style memory allocation stubs that fall back to C runtime
		 *
		 * @param Size					Size in bytes to be allocated
		 * @since Karma 1.0.0
		 */
		static FORCEINLINE void* SystemMalloc(SIZE_T Size)
		{
			/* TODO: Trace! */
			return ::malloc(Size);
		}

		/**
		 * @brief C style memory deallocation
		 *
		 * @param Ptr					Pointer to the location to be deallocated
		 * @since Karma 1.0.0
		 */
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

		/**
		 * @brief Return a zeroed block of allocated memory
		 *
		 * @param Alignment							Not functional
		 * @param Count								Size in bytes to be allocated
		 *
		 * @todo Make Alignment functional in Malloc
		 * @see FMemory::Malloc
		 *
		 * @since Karma 1.0.0
		 */
		static FORCEINLINE void* MallocZeroed(SIZE_T Count, uint32 Alignment = DEFAULT_ALIGNMENT)
		{
			void* Memory = Malloc(Count, Alignment);
			Memzero(Memory, Count);
			return Memory;
		}
	};
}
