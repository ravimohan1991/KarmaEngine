// Copyright Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	UnObjAllocator.h: Unreal object allocation
=============================================================================*/

#pragma once

#include "krpch.h"

namespace Karma
{
	class UObjectBase;

	/**
	 * Traits class which tests if a type is integral.
	 */
	template <typename T>
	struct TIsIntegral
	{
		enum { Value = false };
	};

	template <> struct TIsIntegral<         bool> { enum { Value = true }; };
	template <> struct TIsIntegral<         char> { enum { Value = true }; };
	template <> struct TIsIntegral<signed   char> { enum { Value = true }; };
	template <> struct TIsIntegral<unsigned char> { enum { Value = true }; };
	template <> struct TIsIntegral<         char16_t> { enum { Value = true }; };
	template <> struct TIsIntegral<         char32_t> { enum { Value = true }; };
	template <> struct TIsIntegral<         wchar_t> { enum { Value = true }; };
	template <> struct TIsIntegral<         short> { enum { Value = true }; };
	template <> struct TIsIntegral<unsigned short> { enum { Value = true }; };
	template <> struct TIsIntegral<         int> { enum { Value = true }; };
	template <> struct TIsIntegral<unsigned int> { enum { Value = true }; };
	template <> struct TIsIntegral<         long> { enum { Value = true }; };
	template <> struct TIsIntegral<unsigned long> { enum { Value = true }; };
	template <> struct TIsIntegral<         long long> { enum { Value = true }; };
	template <> struct TIsIntegral<unsigned long long> { enum { Value = true }; };

	template <typename T> struct TIsIntegral<const          T> { enum { Value = TIsIntegral<T>::Value }; };
	template <typename T> struct TIsIntegral<      volatile T> { enum { Value = TIsIntegral<T>::Value }; };
	template <typename T> struct TIsIntegral<const volatile T> { enum { Value = TIsIntegral<T>::Value }; };

	/**
	 * Traits class which tests if a type is a pointer.
	 */
	template <typename T>
	struct TIsPointer
	{
		enum { Value = false };
	};

	template <typename T> struct TIsPointer<T*> { enum { Value = true }; };

	template <typename T> struct TIsPointer<const          T> { enum { Value = TIsPointer<T>::Value }; };
	template <typename T> struct TIsPointer<      volatile T> { enum { Value = TIsPointer<T>::Value }; };
	template <typename T> struct TIsPointer<const volatile T> { enum { Value = TIsPointer<T>::Value }; };

	/**
	 * Aligns a value to the nearest higher multiple of 'Alignment', which must be a power of two.
	 *
	 * @param  Val        The value to align.
	 * @param  Alignment  The alignment value, must be a power of two.
	 *
	 * @return The value aligned up to the specified alignment.
	 */
	template <typename T>
	FORCEINLINE constexpr T Align(T Val, uint64_t Alignment)
	{
		KR_CORE_ASSERT(TIsIntegral<T>::Value || TIsPointer<T>::Value, "Align expects an integer or pointer type");

		return (T)(((uint64_t)Val + Alignment - 1) & ~(Alignment - 1));
	}

	/**
	 * A pool allocator for Karma's UObjects.
	 * I'd higly recomment Gregory's Game Engine Architecture section 5.2 for introductory
	 * level and practical approach to memory system.
	 *
	 * A modular memory system https://github.com/ravimohan1991/cppGameMemorySystem
	 * Karma's take https://github.com/ravimohan1991/KarmaEngine/wiki/Karma-Smriti
	 */
	class FUObjectAllocator
	{
	public:

		/**
		 * Constructor, initializes to no permanent object pool
		 */
		FUObjectAllocator() :
			m_PermanentObjectPoolSize(0),
			m_PermanentObjectPool(nullptr),
			m_PermanentObjectPoolTail(nullptr),
			m_PermanentObjectPoolExceededTail(nullptr)
		{
		}

		/**
		 * Allocates and initializes the permanent object pool. For legacy purpose.
		 *
		 * @param InPermanentObjectPoolSize size of permanent object pool
		 */
		void AllocatePermanentObjectPool(int32_t InPermanentObjectPoolSize);

		/**
		 * Initializes the pool allocator with alloted memory variables
		 *
		 * @param pMemoryStart			Pointer to the start of the pool memory
		 * @param elementSizeBytes		Size of each element in bytes
		 * @param numberOfElements		Tota number of UObjects envisoned
		 * @see   KarmaSmriti::StartUp()
		 */
		void Initialize(uint8_t* pMemoryStart, size_t elemetSizeBytes, size_t numberOfElemets);

		/**
		 * Prints a debugf message to allow tuning
		 */
		void BootMessage();

		/**
		 * Checks whether object is part of permanent object pool.
		 *
		 * @param Object object to test as a member of permanent object pool
		 * @return true if object is part of permanent object pool, false otherwise
		 */
		FORCEINLINE bool ResidesInPermanentPool(const UObjectBase* Object) const
		{
			return ((const uint8_t*)Object >= m_PermanentObjectPool) && ((const uint8_t*)Object < m_PermanentObjectPoolTail);
		}

		/**
		 * Allocates a UObjectBase from the free store or the permanent object pool
		 *
		 * @param Size 									size of uobject to allocate
		 * @param Alignment 							alignment of uobject to allocate
		 * @param bAllowPermanent 						if true, allow allocation in the permanent object pool, if it fits
		 * @return newly allocated UObjectBase (not really a UObjectBase yet, no constructor like thing has been called).
		 */
		UObjectBase* AllocateUObject(size_t Size, size_t Alignment, bool bAllowPermanent);

		/**
		 * Returns a UObjectBase to the free store, unless it is in the permanent object pool
		 *
		 * @param Object object to free
		 */
		//void FreeUObject(UObjectBase* Object) const;

	private:

		/** Size in bytes of pool for objects disregarded for GC.								*/
		int32_t							m_PermanentObjectPoolSize;

		/** Begin of pool for objects disregarded for GC.										*/
		uint8_t* 						m_PermanentObjectPool;

		/** Current position in pool for objects disregarded for GC.							*/
		uint8_t* 						m_PermanentObjectPoolTail;

		/** Tail that exceeded the size of the permanent object pool, >= PermanentObjectPoolTail.		*/
		uint8_t* 						m_PermanentObjectPoolExceededTail;
	};

	/** Global UObjectBase allocator							*/
	extern FUObjectAllocator GUObjectAllocator;
}