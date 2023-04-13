// Copyright Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	UObjectBase.h: Base class for UObject, defines low level functionality
=============================================================================*/

#pragma once

#include "krpch.h"

#include "Core/UObjectGlobals.h"

namespace Karma
{
	//enum EObjectFlags;
	class UObject;
	class UClass;
	//enum class EInternalObjectFlags;
	class UPackage;

	extern std::vector<UObject> GUObjectStore;

/** Mask for all object flags */
#define RF_AllFlags				(EObjectFlags)0xffffffff	///< All flags, used mainly for error checking

	/**
	 * Low level implementation of UObject, should not be used directly in game code
	 * Taken from UE's UObjectBaseUtility.h.
	 */
	class KARMA_API UObjectBase
	{
	protected:
		UObjectBase();

	public:
		/**
		 * Constructor used by StaticAllocateObject
		 * @param	InClass				non NULL, this gives the class of the new object, if known at this time
		 * @param	InFlags				RF_Flags to assign
		 * @param	InOuter				outer for this object
		 * @param	InName				name of the new object
		 * @param	InObjectArchetype	archetype to assign
		 */
		UObjectBase(UClass* inClass, EObjectFlags inFlags, EInternalObjectFlags inInternalFlags, UObject* inOuter, const std::string& inName);

		/**
		 * Walks up the list of outers until it finds a package directly associated with the object.
		 *
		 * @return the package the object is in.
		 */
		UPackage* GetPackage() const;

		/** Returns the external UPackage associated with this object, if any */
		UPackage* GetExternalPackage() const;

	private:

		/** Flags used to track and report various object states. This needs to be 8 byte aligned on 32-bit
			platforms to reduce memory waste */
		EObjectFlags					m_ObjectFlags;

		/** Index into GUObjectStore...very private. */
		int32_t							m_InternalIndex;

		/** Object this object resides in. */
		UObject* m_OuterPrivate;

		/** Class the object belongs to. */
		UClass* m_ClassPrivate;

		/** Name of this object */
		std::string							m_NamePrivate;

	private:
		/**
		 * Add a newly created object to the name hash tables and the object array
		 *
		 * @param name					name to assign to this uobject
		 * @param inSetInternalFlags	Internal object flags to be set on the object once it's been added to the array
		 */
		void AddObject(const std::string& name, EInternalObjectFlags inSetInternalFlags);

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
			static T someObject;
			return (T*)GetTypedOuter(T::StaticClass(&someObject));
		}

		/**
		 * Checks to see if the object appears to be valid
		 * @return true if this appears to be a valid object
		 */
		bool IsValidLowLevel() const;

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
				Flags
		-------------------*/

		/**
		* Retrieve the object flags directly
		*
		* @return Flags for this object
		**/
		FORCEINLINE EObjectFlags GetFlags() const
		{
			KR_CORE_ASSERT((m_ObjectFlags & ~RF_AllFlags) == 0, "{0} flagged as RF_ALLFlags", GetName());
			return m_ObjectFlags;
		}


		/**
		 * Used to safely check whether any of the passed in flags are set.
		 *
		 * @param FlagsToCheck	Object flags to check for.
		 * @return				true if any of the passed in flags are set, false otherwise  (including no flags passed in).
		 */
		FORCEINLINE bool HasAnyFlags(EObjectFlags FlagsToCheck) const
		{
			KR_CORE_ASSERT(!(FlagsToCheck & (RF_MarkAsNative | RF_MarkAsRootSet)) || FlagsToCheck == RF_AllFlags, "Illegal flags being used"); // These flags can't be used outside of constructors / internal code
			return (GetFlags() & FlagsToCheck) != 0;
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
