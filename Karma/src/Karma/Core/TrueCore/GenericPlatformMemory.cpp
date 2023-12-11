#include "GenericPlatformMemory.h"
#include "Karma/Ganit/KarmaMath.h"

#ifndef RESTRICT
#define RESTRICT __restrict						/* no alias hint */
#endif

namespace Karma
{
	void FGenericPlatformMemory::MemswapGreaterThan8(void* RESTRICT Ptr1, void* RESTRICT Ptr2, SIZE_T Size)
	{
		union PtrUnion
		{
			void* PtrVoid;// 4 or 8 bytes based on architecture
			uint8_t* Ptr8;
			uint16_t* Ptr16;
			uint32_t* Ptr32;
			uint64_t* Ptr64;
			UPTRINT PtrUint;// 32 or 64 bit uint, based on the architecture
		};

		PtrUnion Union1 = { Ptr1 };
		PtrUnion Union2 = { Ptr2 };

		KR_CORE_ASSERT(Union1.PtrVoid && Union2.PtrVoid, "Pointers must be non-null: {0}, {1}", Union1.PtrVoid, Union2.PtrVoid);

		// We may skip up to 7 bytes below, so better make sure that we're swapping more than that
		// (8 is a common case that we also want to inline before we this call, so skip that too)
		KR_CORE_ASSERT(Size > 8, "");

		if (Union1.PtrUint & 1)
		{
			Valswap(*Union1.Ptr8++, *Union2.Ptr8++);
			Size -= 1;
		}
		if (Union1.PtrUint & 2)
		{
			Valswap(*Union1.Ptr16++, *Union2.Ptr16++);
			Size -= 2;
		}
		if (Union1.PtrUint & 4)
		{
			Valswap(*Union1.Ptr32++, *Union2.Ptr32++);
			Size -= 4;
		}

		// NOT FUNCTIONAL YET
		uint32_t CommonAlignment = FMath::Min(FMath::CountTrailingZeros((uint32_t)(Union1.PtrUint - Union2.PtrUint)), 3u);
		switch (CommonAlignment)
		{
		default:
			for (; Size >= 8; Size -= 8)
			{
				Valswap(*Union1.Ptr64++, *Union2.Ptr64++);
			}

		case 2:
			for (; Size >= 4; Size -= 4)
			{
				Valswap(*Union1.Ptr32++, *Union2.Ptr32++);
			}

		case 1:
			for (; Size >= 2; Size -= 2)
			{
				Valswap(*Union1.Ptr16++, *Union2.Ptr16++);
			}

		case 0:
			for (; Size >= 1; Size -= 1)
			{
				Valswap(*Union1.Ptr8++, *Union2.Ptr8++);
			}
		}
	}
}
