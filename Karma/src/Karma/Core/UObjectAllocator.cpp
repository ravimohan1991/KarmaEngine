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
		m_PermanentObjectPoolEnd = m_PermanentObjectPool + size_t(m_PermanentObjectPoolSize);
		m_BareUObjectsSize = 0;
		m_AlignedUObjectsSize = 0;

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

	UObjectBase* FUObjectAllocator::AllocateUObject(size_t Size, size_t Alignment, bool bAllowPermanent)
	{
		// Force alignment to minimal of 16 bytes
		Alignment = FMath::Max<size_t>(16, Alignment);
		//int32_t AlignedSize = Align(Size, Alignment);

		UObjectBase* Result = nullptr;
		bAllowPermanent &= m_PermanentObjectPool != nullptr;

		// is memory available?
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

			// Increment bare UObject size and number of UObjects
			m_BareUObjectsSize += (uint32_t)Size;
			m_NumberOfUObjects++;

			// Record the offset for aligned size and add to total size
			m_AlignedUObjectsSize += uint32_t(AlignedPtr - m_PermanentObjectPoolTail) + (uint32_t)Size;

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
			KR_CORE_ASSERT(false, "Memory requirements exceeded the tentative allotment");
		}

		// ue performs alignment test

		return Result;
	}

	void FUObjectAllocator::RegisterUObjectsStatisticsCallback(FUObjectAllocatorCallback dumpCallback)
	{
		m_DumpingCallbacks.Add(dumpCallback);
	}

	void FUObjectAllocator::DumpUObjectsInformation(void* InObject, const std::string& InName, size_t InSize, size_t InAlignment, UClass* InClass)
	{
		// Iterate through all the registered callbacks
		for (const auto& element : m_DumpingCallbacks)
		{
			element(InObject, InName, InSize, InAlignment, InClass);
		}
	}
}
