#pragma once

#include "krpch.h"

namespace Karma
{
	typedef size_t SIZE_T;

	//---------------------------------------------------------------------
	// Utility for automatically setting up the pointer-sized integer type
	//---------------------------------------------------------------------

	template<typename T32BITS, typename T64BITS, int PointerSize>
	struct SelectIntPointerType
	{
		// nothing here are is it an error if the partial specializations fail
	};

	template<typename T32BITS, typename T64BITS>
	struct SelectIntPointerType<T32BITS, T64BITS, 8>
	{
		// Select the 64 bit type.
		typedef T64BITS TIntPointer;
	};

	template<typename T32BITS, typename T64BITS>
	struct SelectIntPointerType<T32BITS, T64BITS, 4>
	{
		// Select the 32 bit type.
		typedef T32BITS TIntPointer;
	};

	// Unsigned int. The same size as a pointer.
	typedef SelectIntPointerType<uint32_t, uint64_t, sizeof(void*)>::TIntPointer UPTRINT;

	enum class EMemcpyCachePolicy : uint8_t
	{
		// Writes to destination memory are cache-visible (default).
		// This should be used if copy results are immediately accessed by CPU.
		StoreCached,

		// Writes to destination memory bypass cache (avoiding pollution).
		// Optimizes for large copies that aren't read from soon after.
		StoreUncached,
	};

	struct KARMA_API FGenericPlatformMemory
	{
		/**
		 * Copies count bytes of characters from Src to Dest. If some regions of the source
		 * area and the destination overlap, memmove ensures that the original source bytes
		 * in the overlapping region are copied before being overwritten.  NOTE: make sure
		 * that the destination buffer is the same size or larger than the source buffer!
		 */
		static FORCEINLINE void* Memmove(void* Dest, const void* Src, SIZE_T Count)
		{
			return memmove(Dest, Src, Count);
		}

		static FORCEINLINE int32_t Memcmp(const void* Buf1, const void* Buf2, SIZE_T Count)
		{
			return memcmp(Buf1, Buf2, Count);
		}

		static FORCEINLINE void* Memset(void* Dest, uint8_t Char, SIZE_T Count)
		{
			return memset(Dest, Char, Count);
		}

		static FORCEINLINE void* Memzero(void* Dest, SIZE_T Count)
		{
			return memset(Dest, 0, Count);
		}

		static FORCEINLINE void* Memcpy(void* Dest, const void* Src, SIZE_T Count)
		{
			return memcpy(Dest, Src, Count);
		}

		/** Memcpy optimized for big blocks. */
		static FORCEINLINE void* BigBlockMemcpy(void* Dest, const void* Src, SIZE_T Count)
		{
			return memcpy(Dest, Src, Count);
		}

		/** On some platforms memcpy optimized for big blocks that avoid L2 cache pollution are available */
		static FORCEINLINE void* StreamingMemcpy(void* Dest, const void* Src, SIZE_T Count)
		{
			return memcpy(Dest, Src, Count);
		}

		/** On some platforms memcpy can be distributed over multiple threads for throughput. */
		static FORCEINLINE void* ParallelMemcpy(void* Dest, const void* Src, SIZE_T Count, EMemcpyCachePolicy Policy = EMemcpyCachePolicy::StoreCached)
		{
			(void)Policy;
			return memcpy(Dest, Src, Count);
		}

	private:
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
