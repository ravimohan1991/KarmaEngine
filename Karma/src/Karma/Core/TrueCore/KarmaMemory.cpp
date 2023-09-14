#include "KarmaMemory.h"

namespace Karma
{
	// We are not making use of alignment at the moment
	void* FMemory::Malloc(SIZE_T Count, uint32 Alignment)
	{
		static void* returnPointer = malloc(Count);
		
		return returnPointer;
	}
}