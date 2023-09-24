#include "KarmaSmriti.h"
#include "KarmaMemory.h"

#include "Core/UObjectAllocator.h"

namespace Karma
{
	KarmaSmriti::KarmaSmriti()
	{
	}

	KarmaSmriti::~KarmaSmriti()
	{
	}

	void KarmaSmriti::StartUp()
	{
		KR_CORE_INFO("Initializing Karma's Memory System");

		// Do some minor computation of number of UObjects allowed
		const size_t poolBytes = 256 * 50;

		// Allocate memory region for all allocators
		m_TotalBytes = poolBytes;//dynamicBytes + persistantBytes + oneFrameBytes + pool16Bytes + pool32Bytes;
		m_pMemBlock = FMemory::SystemMalloc(m_TotalBytes);

		uint8_t* pBytePtr = static_cast<uint8_t*>(m_pMemBlock);
		GUObjectAllocator.Initialize(pBytePtr, 256, 50);
	}

	void KarmaSmriti::ShutDown()
	{
		FMemory::SystemFree(m_pMemBlock);
		KR_CORE_INFO("Freed Karma's memory softbed");
	}
}
