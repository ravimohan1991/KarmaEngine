/**
 * @file ObjectBase.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the class UObjectBase class.
 * @version 1.0
 * @date March 1, 2023
 *
 * @copyright Karma Engine copyright(c) People of India, in accordance with Epic EULA (https://www.unrealengine.com/en-US/eula/unreal)
 */

/*=============================================================================
	UObjectBase.h: Base class for UObject, defines low level functionality
=============================================================================*/

#pragma once

#include "Core/UObjectGlobals.h"

namespace Karma
{
	class UObject;
	class UClass;

	class UPackage;

	/** Mask for all object flags */
	#define RF_AllFlags				(EObjectFlags)0xffffffff	///< All flags, used mainly for error checking

	/**
	 * @brief Low level implementation of UObject, should not be used directly in game code
	 * Taken from UE's UObjectBaseUtility.h.
	 */
	class KARMA_API UObjectBase
	{
	protected:
		UObjectBase();

		/**
		 * Set the object flags directly
		 *
		 **/
		FORCEINLINE void SetFlagsTo(EObjectFlags NewFlags)
		{
			KR_CORE_ASSERT((NewFlags & ~RF_AllFlags) == 0, "%s flagged as 0x%x but is trying to set flags to RF_AllFlags");
			m_ObjectFlags = NewFlags;
		}

	public:
		/**
		 * Constructor used by StaticAllocateObject
		 * @param	InClass				non NULL, this gives the class of the new object, if known at this time
		 * @param	InFlags				RF_Flags to assign
		 * @param	InOuter				outer for this object
		 * @param	InName				name of the new object
		 * @param	InObjectArchetype	archetype to assign (not functional for now)
		 * @see StaticAllocateObject() in UObjectGlobals.h
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

		/** Checks if the object is unreachable. */
		// Caution: use with vigilance
		bool IsUnreachable() const;

		/** Set indexing for GUObjectStore for lookup of UObjects. */
		FORCEINLINE void SetInternalIndex(uint32_t StoreIndex) { m_InternalIndex = StoreIndex; }

		FORCEINLINE uint32_t GetInterIndex() const { return m_InternalIndex; }

	private:

		/** Flags used to track and report various object states. This needs to be 8 byte aligned on 32-bit
			platforms to reduce memory waste */
		EObjectFlags					m_ObjectFlags;

		/**
		 * Index into GUObjectStore... very private.
		 * Since this is meant literally for array index, hence we take positive data structure
		 */
		uint32_t							m_InternalIndex;

		/** Object this object resides in. */
		UObject* m_OuterPrivate;

		/** Class the object belongs to. */
		UClass* m_ClassPrivate;

		/** Name of this object */
		std::string							m_NamePrivate;

		/** If true, objects will never be marked as PendingKill so references to them will not be nulled automatically by the garbage collector */
		static bool m_bPendingKillDisabled;

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

		FORCEINLINE void SetObjectName(const std::string& aName)
		{
			m_NamePrivate = aName;
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

		/**
		 * Checks to see if the object appears to be valid
		 * @return true if this appears to be a valid object
		 */
		bool IsValidLowLevel() const;

		/** Helper function that sets the appropriate flag based on PK being enabled or not */
		FORCEINLINE static EInternalObjectFlags FixGarbageOrPendingKillInternalObjectFlags(const EInternalObjectFlags InFlags)
		{
			//PRAGMA_DISABLE_DEPRECATION_WARNINGS
			if (!(int32_t(InFlags) & (int32_t(EInternalObjectFlags::Garbage) | int32_t(EInternalObjectFlags::PendingKill))))
			{
				// Pass through
				return InFlags;
			}
			else
			{
				return m_bPendingKillDisabled ?
					EInternalObjectFlags(((int32_t(InFlags) & ~int32_t(EInternalObjectFlags::PendingKill)) | int32_t(EInternalObjectFlags::Garbage))) : // Replace PK with Garbage
					EInternalObjectFlags(((int32_t(InFlags) & ~int32_t(EInternalObjectFlags::Garbage)) | int32_t(EInternalObjectFlags::PendingKill))); // Replace Garbage with PK
			}
			//PRAGMA_ENABLE_DEPRECATION_WARNINGS
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

			KR_CORE_ASSERT(ThisClass, "Comparing with null is useless");

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
		 * Modifies object flags for a specific object
		 */
		FORCEINLINE void SetFlags(EObjectFlags NewFlags)
		{
			KR_CORE_ASSERT(!(NewFlags & (RF_MarkAsNative | RF_MarkAsRootSet | RF_PendingKill | RF_Garbage)), "These flags can't be used outside of constructors / internal code");
			KR_CORE_ASSERT(!(NewFlags & (EObjectFlags)(RF_PendingKill | RF_Garbage)) || (GetFlags() & (NewFlags & (EObjectFlags)(RF_PendingKill | RF_Garbage))) == (NewFlags & (EObjectFlags)(RF_PendingKill | RF_Garbage)), "RF_PendingKill and RF_garbage can not be set through SetFlags function. Use MarkAsGarbage() instead");

			SetFlagsTo(EObjectFlags (GetFlags() | NewFlags));
		}

		/** Clears subset of flags for a specific object */
		FORCEINLINE void ClearFlags(EObjectFlags NewFlags)
		{
			KR_CORE_ASSERT(!(NewFlags & (RF_MarkAsNative | RF_MarkAsRootSet | RF_PendingKill | RF_Garbage)) || NewFlags == RF_AllFlags, "These flags can't be used outside of constructors / internal code");
			KR_CORE_ASSERT(!(NewFlags & (EObjectFlags)(RF_PendingKill | RF_Garbage)) || (GetFlags() & (NewFlags & (EObjectFlags)(RF_PendingKill | RF_Garbage))) == RF_NoFlags, "RF_PendingKill and RF_garbage can not be cleared through ClearFlags function. Use ClearGarbage() instead");

			SetFlagsTo(EObjectFlags (GetFlags() & ~NewFlags));
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

		/**
		 * Used to safely check whether any of the passed in internal flags are set.
		 *
		 * @param FlagsToCheck	Object flags to check for.
		 * @return				true if any of the passed in flags are set, false otherwise  (including no flags passed in).
		 */
		FORCEINLINE bool HasAnyInternalFlags(EInternalObjectFlags FlagsToCheck) const
		{
			return GUObjectStore.IndexToObject(m_InternalIndex)->HasAnyFlags(FlagsToCheck);
		}

		/**
		 * Sets internal flags of the FUObjectItem
		 *
		 * @param FlagsToSet
		 */
		FORCEINLINE void SetInternalFlags(EInternalObjectFlags FlagsToSet) const
		{
			FUObjectItem* ObjectItem = GUObjectStore.IndexToObject(m_InternalIndex);
			KR_CORE_ASSERT(!(int32_t(FlagsToSet) & (int32_t(EInternalObjectFlags::PendingKill) | int32_t(EInternalObjectFlags::Garbage))) || (int32_t(FlagsToSet) & (int32_t(EInternalObjectFlags::PendingKill) | int32_t(EInternalObjectFlags::Garbage))) == (int32_t(ObjectItem->GetFlags()) & (int32_t(EInternalObjectFlags::PendingKill) | int32_t(EInternalObjectFlags::Garbage))), "");

			ObjectItem->SetFlags(FlagsToSet);
		}

		/**
		 * Add an object to the root set. This prevents the object and all
		 * its descendants from being deleted during garbage collection.
		 */
		FORCEINLINE void AddToRoot()
		{
			// Need to understand the logic of GC first
			//GUObjectStore.IndexToObject(InternalIndex)->SetRootSet();
		}

		/*-------------------
			Class
		-------------------*/

	private:

		/**
		 * See if ObjClass is child of TestClass
		 */
		static bool IsChildOfWorkaround(const UClass* ObjClass, const UClass* TestClass);

		//static FORCEINLINE bool IsChildOfWorkaround(const ClassType* ObjClass, const ClassType* TestCls);
		//{
		//	return ObjClass->IsChildOf(TestCls);
		//}
	};
}
