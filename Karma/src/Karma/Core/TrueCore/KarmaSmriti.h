/**
 * @file KarmaSmriti.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the class KarmaSmriti, an agnostic memory manager.
 * @version 1.0
 * @date April 27, 2023
 *
 * @copyright Karma Engine copyright(c) People of India
 */

#pragma once

#include "krpch.h"

#include "Core/UObjectAllocator.h"

namespace Karma
{
	/**
	 * @brief Karma's memory management.
	 * Heavily influenced from https://github.com/ravimohan1991/cppGameMemorySystem and Unreal Engine
	 */
	class KARMA_API KarmaSmriti
	{
	public:
		/**
		 * @brief Constructor
		 *
		 * @since Karma 1.0.0
		 */
		KarmaSmriti();

		/**
		 * @brief Destructor
		 *
		 * @since Karma 1.0.0
		 */
		~KarmaSmriti();

		/**
		 * @brief Allocates the memory block and sets up the memory system for use.
		 *
		 * @since Karma 1.0.0
		 */
		void StartUp();

		/**
		 * @brief Releases memory & any other resources held by the memory system (KarmaSmriti) and allocators.
		 *
		 * @since Karma 1.0.0
		 */
		void ShutDown();

		/**
		 * @brief Getter for the pointer to start of allocated memory address
		 *
		 * @since Karma 1.0.0
		 */
		void* GetMemBlock() const { return m_pMemBlock; }

	private:
		/**
		 * @brief Pointer to the start of allocated memory
		 *
		 * @since Karma 1.0.0
		 */
		void* m_pMemBlock = nullptr;

		/**
		 * @brief Size of the entire memory block used by all allocators in bytes
		 *
		 * @since Karma 1.0.0
		 */
		size_t m_TotalBytes = 0;

	};
}
