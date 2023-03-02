// Copyright Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	UObjectBase.h: Base class for UObject, defines low level functionality
=============================================================================*/

#pragma once

#include "krpch.h"

namespace Karma
{
	enum EObjectFlags;
	class UObject;
	class UClass;

	/**
	 * Low level implementation of UObject, should not be used directly in game code
	 * Taken from UE's UObjectBaseUtility.h.
	 */
	class KARMA_API UObjectBase
	{
	private:

		/** Flags used to track and report various object states. This needs to be 8 byte aligned on 32-bit
			platforms to reduce memory waste */
		EObjectFlags					m_ObjectFlags;

		/** Index into GObjectArray...very private. */
		int32_t							m_InternalIndex;

		/** Object this object resides in. */
		UObject* m_OuterPrivate;

		/** Class the object belongs to. */
		UClass* m_ClassPrivate;

		/** Name of this object */
		std::string							m_NamePrivate;

	public:
		/**
		 * Marks the object for garbage collection
		 */
		void MarkAsGarbage();

		/**
		 * Unmarks this object as Garbage.
		 */
		void ClearGarbage();

		/** Returns the logical name of this object */
		FORCEINLINE const std::string& GetName() const
		{
			return m_NamePrivate;
		}

		/** Returns the UObject this object resides in */
		FORCEINLINE UObject* GetOuter() const
		{
			return m_OuterPrivate;
		}

		/**
		 * Traverses the outer chain searching for the next object of a certain type.  (T must be derived from UObject)
		 *
		 * @param	Target class to search for
		 * @return	a pointer to the first object in this object's Outer chain which is of the correct type.
		 */
		UObject* GetTypedOuter(UClass* Target) const;

		/**
		 * Traverses the outer chain searching for the next object of a certain type.  (T must be derived from UObject)
		 *
		 * @return	a pointer to the first object in this object's Outer chain which is of the correct type.
		 */
		template<typename T>
		T* GetTypedOuter() const
		{
			return (T*)GetTypedOuter(T::StaticClass());
		}

	public:
		/** Returns true if this object is of the specified type. */
		template <typename OtherClassType>
		FORCEINLINE bool IsA(OtherClassType SomeBase) const
		{
			// We have a cyclic dependency between UObjectBase and UClass,
			// so we use a template to allow inlining of something we haven't yet seen, because it delays compilation until the function is called.

			// 'static_assert' that this thing is actually a UClass pointer or convertible to it.
			const UClass* SomeBaseClass = SomeBase;
			(void)SomeBaseClass;
			KR_CORE_ASSERT(SomeBaseClass, "IsA(NULL) cannot yield meaningful results");

			const UClass* ThisClass = GetClass();

			// Stop the compiler doing some unnecessary branching for nullptr checks
			// Would be interesting to write the analog of this
			//UE_ASSUME(SomeBaseClass);
			//UE_ASSUME(ThisClass);

			return IsChildOfWorkaround(ThisClass, SomeBaseClass);
		}

		/** Returns the UClass that defines the fields of this object */
		FORCEINLINE UClass* GetClass() const
		{
			return m_ClassPrivate;
		}

		/*-------------------
			Class
		-------------------*/

	private:
		// Hmm, let me try my way
		//template <typename ClassType>
		static bool IsChildOfWorkaround(const UClass* ObjClass, const UClass* TestCls);
		//static FORCEINLINE bool IsChildOfWorkaround(const ClassType* ObjClass, const ClassType* TestCls);
		//{
		//	return ObjClass->IsChildOf(TestCls);
		//}
	};
}