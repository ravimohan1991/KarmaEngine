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
		m_PermanentObjectPoolSize = InPermanentObjectPoolSize;
		m_PermanentObjectPool = (uint8_t*)FMemory::SystemMalloc(InPermanentObjectPoolSize);//MallocPersistentAuxiliary(PermanentObjectPoolSize);
		m_PermanentObjectPoolTail = m_PermanentObjectPool;
		m_PermanentObjectPoolExceededTail = m_PermanentObjectPoolTail;
	}

	void FUObjectAllocator::Initialize(uint8_t* pMemoryStart, size_t elemetSizeBytes, size_t numberOfElemets)
	{
		m_PermanentObjectPoolSize = (int32_t) (elemetSizeBytes * numberOfElemets);
		m_PermanentObjectPool = pMemoryStart;
		m_PermanentObjectPoolTail = m_PermanentObjectPool;
		m_PermanentObjectPoolExceededTail = m_PermanentObjectPoolTail;

		KR_CORE_INFO("Prepared a memory pool of {0} bytes for {1} UObjects' allocation", m_PermanentObjectPoolSize, numberOfElemets);
	}

	/**
	 * Prints a debugf message to allow tuning
	 */
	void FUObjectAllocator::BootMessage()
	{
		if (m_PermanentObjectPoolSize && m_PermanentObjectPoolExceededTail - m_PermanentObjectPool > m_PermanentObjectPoolSize)
		{
			KR_CORE_WARN("{0} Exceeds size of permanent object pool {1}, please tune SizeOfPermanentObjectPool.", m_PermanentObjectPoolExceededTail - m_PermanentObjectPool, m_PermanentObjectPoolSize);
		}
		else
		{
			KR_CORE_WARN("{0} out of {1} bytes used by permanent object pool ", m_PermanentObjectPoolExceededTail - m_PermanentObjectPool, m_PermanentObjectPoolSize);
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

		bAllowPermanent &= m_PermanentObjectPool != nullptr;
		const bool bPlaceInPerm = bAllowPermanent && (Align(m_PermanentObjectPoolTail, Alignment) + Size) <= (m_PermanentObjectPool + m_PermanentObjectPoolSize);
		if (bAllowPermanent && !bPlaceInPerm)
		{
			// advance anyway so we can determine how much space we should set aside in the ini
			uint8_t* AlignedPtr = Align(m_PermanentObjectPoolExceededTail, Alignment);
			m_PermanentObjectPoolExceededTail = AlignedPtr + Size;
		}
		// Use object memory pool for objects disregarded by GC (initially loaded ones). This allows identifying their
		// GC status by simply looking at their address.
		if (bPlaceInPerm)
		{
			// Align current tail pointer and use it for object. 
			uint8_t* AlignedPtr = Align(m_PermanentObjectPoolTail, Alignment);
			// Update tail pointer.
			m_PermanentObjectPoolTail = AlignedPtr + Size;
			Result = (UObjectBase*)AlignedPtr;
			if (m_PermanentObjectPoolExceededTail < m_PermanentObjectPoolTail)
			{
				m_PermanentObjectPoolExceededTail = m_PermanentObjectPoolTail;
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
