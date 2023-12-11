/**
 * @file GenericPlatformMemory.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the FGenericPlatformMemory for general purpose memory operations.
 * @version 1.0
 * @date March 8, 2023
 *
 * @copyright Karma Engine copyright(c) People of India
 */

#pragma once

#include "krpch.h"

namespace Karma
{
	typedef size_t SIZE_T;

	//---------------------------------------------------------------------
	// Utility for automatically setting up the pointer-sized integer type
	//---------------------------------------------------------------------

	/**
	 * @brief Defaulter for sized different from 4 and 8. Meaning not supporting 16 bit systems
	 *
	 * @see SelectIntPointerType
	 * @since Karma 1.0.0
	 */
	template<typename T32BITS, typename T64BITS, int PointerSize>
	struct SelectIntPointerType
	{
		// nothing here are is it an error if the partial specializations fail
	};

	/**
	 * @brief For selecting 64 bit type
	 *
	 * @see SelectIntPointerType
	 * @since Karma 1.0.0
	 */
	template<typename T32BITS, typename T64BITS>
	struct SelectIntPointerType<T32BITS, T64BITS, 8>
	{
		// Select the 64 bit type.
		typedef T64BITS TIntPointer;
	};

	/**
	 * @brief For selecting 32 bit type
	 *
	 * @see SelectIntPointerType
	 * @since Karma 1.0.0
	 */
	template<typename T32BITS, typename T64BITS>
	struct SelectIntPointerType<T32BITS, T64BITS, 4>
	{
		// Select the 32 bit type.
		typedef T32BITS TIntPointer;
	};

	/**
	 * A typedef for selecting between 32 bit and 64 bit uint based upon the architecture
	 *
	 * @note In 16-bit systems, the size of a void pointer is 2 bytes. In a 32-bit system, the size of a
	 * void pointer is 4 bytes. And, in a 64-bit system, the size of a void pointer is 8 bytes.
	 *
	 * @since Karma 1.0.0
	 */
	typedef SelectIntPointerType<uint32_t, uint64_t, sizeof(void*)>::TIntPointer UPTRINT;

	enum class EMemcpyCachePolicy : uint8_t
	{
		/**
		 * @brief Writes to destination memory are cache-visible (default).
		 *
		 * @remark This should be used if copy results are immediately accessed by CPU.
		 */
		StoreCached,

		/**
		 * @brief Writes to destination memory bypass cache (avoiding pollution).
		 *
		 * @remark Optimizes for large copies that aren't read from soon after.
		 */
		StoreUncached,
	};

	/**
	 * @brief Base class for Platform based memory operations
	 */
	struct KARMA_API FGenericPlatformMemory
	{
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
			return memmove(Dest, Src, Count);
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
			return memcmp(Buf1, Buf2, Count);
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
			return memset(Dest, Char, Count);
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
			return memset(Dest, 0, Count);
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
			return memcpy(Dest, Src, Count);
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
			return memcpy(Dest, Src, Count);
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
			return memcpy(Dest, Src, Count);
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
			(void)Policy;
			return memcpy(Dest, Src, Count);
		}

	private:
		/**
		 * @brief Swap the value of supplied variables
		 *
		 * @param A				A variable
		 * @param B				Another variable
		 *
		 * @since Karma 1.0.0
		 */
		template <typename T>
		static FORCEINLINE void Valswap(T& A, T& B)
		{
			// Usually such an implementation would use move semantics, but
			// we're only ever going to call it on fundamental types and MoveTemp
			// is not necessarily in scope here anyway, so we don't want to
			// #include it if we don't need to.
			T Tmp = A;
			A = B;
			B = Tmp;
		}

		/**
		 * @brief For swapping values, pointed by pointer, in some sort of clustering scheme
		 *
		 * @param Ptr1
		 * @param Ptr2
		 *
		 * @param Size Amount of data to be displaced
		 *
		 * @note Not completely functional. Need to find a use case first and then proceed.
		 * @since Karma 1.0.0
		 */
		static void MemswapGreaterThan8(void* Ptr1, void* Ptr2, SIZE_T Size);

	public:
		// NOT FUNCTIONAL YET
		static inline void Memswap(void* Ptr1, void* Ptr2, SIZE_T Size)
		{
			switch (Size)
			{
			case 0:
				break;

			case 1:
				Valswap(*(uint8_t*)Ptr1, *(uint8_t*)Ptr2);
				break;

			case 2:
				Valswap(*(uint16_t*)Ptr1, *(uint16_t*)Ptr2);
				break;

			case 3:
				Valswap(*((uint16_t*&)Ptr1)++, *((uint16_t*&)Ptr2)++);
				Valswap(*(uint8_t*)Ptr1, *(uint8_t*)Ptr2);
				break;

			case 4:
				Valswap(*(uint32_t*)Ptr1, *(uint32_t*)Ptr2);
				break;

			case 5:
				Valswap(*((uint32_t*&)Ptr1)++, *((uint32_t*&)Ptr2)++);
				Valswap(*(uint8_t*)Ptr1, *(uint8_t*)Ptr2);
				break;

			case 6:
				Valswap(*((uint32_t*&)Ptr1)++, *((uint32_t*&)Ptr2)++);
				Valswap(*(uint16_t*)Ptr1, *(uint16_t*)Ptr2);
				break;

			case 7:
				Valswap(*((uint32_t*&)Ptr1)++, *((uint32_t*&)Ptr2)++);
				Valswap(*((uint16_t*&)Ptr1)++, *((uint16_t*&)Ptr2)++);
				Valswap(*(uint8_t*)Ptr1, *(uint8_t*)Ptr2);
				break;

			case 8:
				Valswap(*(uint64_t*)Ptr1, *(uint64_t*)Ptr2);
				break;

			case 16:
				Valswap(((uint64_t*)Ptr1)[0], ((uint64_t*)Ptr2)[0]);
				Valswap(((uint64_t*)Ptr1)[1], ((uint64_t*)Ptr2)[1]);
				break;

			default:
				MemswapGreaterThan8(Ptr1, Ptr2, Size);
				break;
			}
		}
	};
}
