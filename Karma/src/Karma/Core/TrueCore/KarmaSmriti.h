#pragma once

#include "krpch.h"

#include "Core/UObjectAllocator.h"

namespace Karma
{
	/**
	 * Karma's memory management
	 * Heavily influenced from https://github.com/ravimohan1991/cppGameMemorySystem
	 */
	class KARMA_API KarmaSmriti
	{
	public:
		KarmaSmriti();
		~KarmaSmriti();

		/**
		 * Allocates the memory block and sets up the memory system for use.
		 *
		 */
		void StartUp();

		/**
		 * Releases memory & any other resources held by the memory system (KarmaSmriti) and allocators
		 *
		 */
		void ShutDown();

	private:
		/**
		 * Pointer to the start of allocated memory
		 */
		void* m_pMemBlock = nullptr;

		/**
		 * Size of the entire memory block used by all allocators in bytes
		 */
		size_t m_TotalBytes = 0;

	};
}
