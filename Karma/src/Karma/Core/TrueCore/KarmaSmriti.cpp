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
		const size_t poolBytes = 256 * 50000;

		// Allocate memory region for all allocators
		size_t m_totalBytes = poolBytes;//dynamicBytes + persistantBytes + oneFrameBytes + pool16Bytes + pool32Bytes;
		m_pMemBlock = FMemory::SystemMalloc(m_totalBytes);

		uint8_t* pBytePtr = static_cast<uint8_t*>(m_pMemBlock);
		GUObjectAllocator.Initialize(pBytePtr, 256, 50000);
	}

	void KarmaSmriti::ShutDown()
	{
		free(m_pMemBlock);
		KR_CORE_INFO("Freed Karma's memory softbed");
	}
}
