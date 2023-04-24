#include "UObjectAllocator.h"
#include "Karma/Core/TrueCore/KarmaMemory.h"
#include "Karma/Ganit/KarmaMath.h"
#include "Core/UObjectBase.h"

namespace Karma
{
	/** Global UObjectBase allocator							*/
	FUObjectAllocator GUObjectAllocator;

	/**
	 * Allocates and initializes the permanent object pool
	 *
	 * @param InPermanentObjectPoolSize size of permanent object pool
	 */
	void FUObjectAllocator::AllocatePermanentObjectPool(int32_t InPermanentObjectPoolSize)
	{
		PermanentObjectPoolSize = InPermanentObjectPoolSize;
		PermanentObjectPool = (uint8_t*)FMemory::SystemMalloc(InPermanentObjectPoolSize);//MallocPersistentAuxiliary(PermanentObjectPoolSize);
		PermanentObjectPoolTail = PermanentObjectPool;
		PermanentObjectPoolExceededTail = PermanentObjectPoolTail;
	}

	/**
	 * Prints a debugf message to allow tuning
	 */
	void FUObjectAllocator::BootMessage()
	{
		if (PermanentObjectPoolSize && PermanentObjectPoolExceededTail - PermanentObjectPool > PermanentObjectPoolSize)
		{
			KR_CORE_WARN("{0} Exceeds size of permanent object pool {1}, please tune SizeOfPermanentObjectPool.", PermanentObjectPoolExceededTail - PermanentObjectPool, PermanentObjectPoolSize);
		}
		else
		{
			KR_CORE_WARN("{0} out of {1} bytes used by permanent object pool ", PermanentObjectPoolExceededTail - PermanentObjectPool, PermanentObjectPoolSize);
		}
	}

	/**
	 * Allocates a UObjectBase from the free store or the permanent object pool
	 *
	 * @param Size size of uobject to allocate
	 * @param Alignment alignment of uobject to allocate
	 * @param bAllowPermanent if true, allow allocation in the permanent object pool, if it fits
	 * @return newly allocated UObjectBase (not really a UObjectBase yet, no constructor like thing has been called).
	 */
	UObjectBase* FUObjectAllocator::AllocateUObject(int32_t Size, int32_t Alignment, bool bAllowPermanent)
	{
		// Force alignment to minimal of 16 bytes
		Alignment = FMath::Max(16, Alignment);
		int32_t AlignedSize = Align(Size, Alignment);
		UObjectBase* Result = nullptr;

		bAllowPermanent &= PermanentObjectPool != nullptr;
		const bool bPlaceInPerm = bAllowPermanent && (Align(PermanentObjectPoolTail, Alignment) + Size) <= (PermanentObjectPool + PermanentObjectPoolSize);
		if (bAllowPermanent && !bPlaceInPerm)
		{
			// advance anyway so we can determine how much space we should set aside in the ini
			uint8_t* AlignedPtr = Align(PermanentObjectPoolExceededTail, Alignment);
			PermanentObjectPoolExceededTail = AlignedPtr + Size;
		}
		// Use object memory pool for objects disregarded by GC (initially loaded ones). This allows identifying their
		// GC status by simply looking at their address.
		if (bPlaceInPerm)
		{
			// Align current tail pointer and use it for object. 
			uint8_t* AlignedPtr = Align(PermanentObjectPoolTail, Alignment);
			// Update tail pointer.
			PermanentObjectPoolTail = AlignedPtr + Size;
			Result = (UObjectBase*)AlignedPtr;
			if (PermanentObjectPoolExceededTail < PermanentObjectPoolTail)
			{
				PermanentObjectPoolExceededTail = PermanentObjectPoolTail;
			}
		}
		else
		{
			// Allocate new memory of the appropriate size and alignment.
			Result = (UObjectBase*)FMemory::Malloc(Size, Alignment);
		}

		return Result;
	}
}
