/**
 * @file UObjectBase.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the class UObjectBase class.
 * @version 1.0
 * @date March 1, 2023
 *
 * @copyright Karma Engine copyright(c) People of India
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
	 * 
	 * Taken from UE's UObjectBaseUtility.h and combined UObjectBase and UObjectBaseUtility classes
	 * for simplicity
	 */
	class KARMA_API UObjectBase
	{
	protected:
		/**
		 * @brief Providing a default constructor
		 * 
		 * Basically required constructor for DECLARE_KARMA_CLASS(TClass, TSuperClass)
		 * in the derived classes
		 * 
		 * @see UObject::UObject()
		 * @since Karma 1.0.0
		 */
		UObjectBase();

		/**
		 * Set the object flags directly
		 * 
		 * @see UObjectBase::SetFlags()
		 * @see UObjectBase::ClearFlags()
		 * 
		 * @since Karma 1.0.0
		 */
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
		 * @param	InOuter				outer for this object (UObject this object resides in, for instance LevelToSpawnIn is outer for AActor)
		 * @param	InName				name of the new object
		 * @param	InObjectArchetype	archetype to assign (not functional for now)
		 * @see StaticAllocateObject() in UObjectGlobals.h
		 * @since Karma 1.0.0
		 */
		UObjectBase(UClass* inClass, EObjectFlags inFlags, EInternalObjectFlags inInternalFlags, UObject* inOuter, const std::string& inName);

		/**
		 * Walks up the list of outers until it finds a package directly associated with the object.
		 *
		 * @return the package the object is in
		 * @since Karma 1.0.0
		 */
		UPackage* GetPackage() const;

		/** 
		 * Returns the external UPackage associated with this object, if any 
		 * @since Karma 1.0.0
		 */
		UPackage* GetExternalPackage() const;

		/** 
		 * Checks if the object is unreachable.
		 *
		 * @warning use with vigilance
		 * @since Karma 1.0.0
		 */
		bool IsUnreachable() const;

		/** 
		 * Set indexing for GUObjectStore for lookup of UObjects.
		 * 
		 * @see FUObjectArray::AddUObject
		 */
		FORCEINLINE void SetInternalIndex(uint32_t StoreIndex) { m_InternalIndex = StoreIndex; }

		/**
		 * Getter for m_InternalIndex of UObjects
		 * 
		 * @see TentativeFlagChecks()
		 */
		FORCEINLINE uint32_t GetInternalIndex() const { return m_InternalIndex; }

	private:

		/**
		 * Flags used to track and report various object states. This needs to be 8 byte aligned on 32-bit
		 * platforms to reduce memory waste (UE's recommendation)
		 * 
		 * @remark We are not yet aligning this member because we need to first understand the meaning in this context
		 */
		EObjectFlags					m_ObjectFlags;

		/**
		 * Index into GUObjectStore... very private.
		 * 
		 * @see UObjectBase::GetInternalIndex
		 * @remark Since this is meant literally for array index, hence we take positive data structure
		 */
		uint32_t							m_InternalIndex;

		/** 
		 * Object this object resides in. 
		 * 
		 * @remark Could be LevelToSpawnIn for AActor or UPackage for UWorld
		 * @see UWorld::CreateWorld
		 */
		UObject* m_OuterPrivate;

		/** Class the object belongs to. */
		UClass* m_ClassPrivate;

		/** Name of this object */
		std::string							m_NamePrivate;

		/** If true, objects will never be marked as PendingKill so references to them will not be nulled automatically by the garbage collector */
		static bool m_bPendingKillDisabled;

	private:
		/**
		 * @brief Add a newly created object to the name hash tables and the object array
		 *
		 * Add a newly created object to the name hash tables (for now no hashtables for Karma) and the object array
		 *
		 * @param name					name to assign to this uobject
		 * @param inSetInternalFlags	Internal object flags to be set on the object once it's been added to the array
		 *
		 * @todo Write a decent hashing class
		 * @since Karma 1.0.0
		 */
		void AddObject(const std::string& name, EInternalObjectFlags inSetInternalFlags);

	public:
		/**
		 * Marks the object for garbage collection
		 * 
		 * @todo Not functional
		 * @since Karma 1.0.0
		 */
		void MarkAsGarbage();

		/**
		 * Unmarks this object as Garbage
		 * 
		 * @todo Not functional
		 * @since Karma 1.0.0
		 */
		void ClearGarbage();

		/**
		 * Returns the logical name of this object 
		 * 
		 * @since Karma 1.0.0
		 */
		FORCEINLINE const std::string& GetName() const
		{
			return m_NamePrivate;
		}

		/**
		 * Sets the name of UObjects
		 * 
		 * @since Karma 1.0.0
		 */
		FORCEINLINE void SetObjectName(const std::string& aName)
		{
			m_NamePrivate = aName;
		}

		/** 
		 * Returns the UObject this object resides in 
		 * 
		 * @remark Could be LevelToSpawnIn for AActor or UPackage for UWorld
		 * @see UWorld::CreateWorld for how UPackage is outer for UWorld
		 * 
		 * @since Karma 1.0.0
		 */
		FORCEINLINE UObject* GetOuter() const
		{
			return m_OuterPrivate;
		}

		/**
		 * Traverses the outer chain searching for the next object of a certain type.  (T must be derived from UObject)
		 *
		 * @param	Target class to search for
		 * @return	a pointer to the first object in this object's Outer chain which is of the correct type.
		 * 
		 * @since Karma 1.0.0
		 */
		UObject* GetTypedOuter(UClass* Target) const;

		/**
		 * Traverses the outer chain searching for the next object of a certain type.
		 *
		 * @return	a pointer to the first object in this object's Outer chain which is of the correct type.
		 * @remark	T must be derived from UObject
		 * 
		 * @since Karma 1.0.0
		 */
		template<typename T>
		T* GetTypedOuter() const
		{
			return (T*)GetTypedOuter(T::StaticClass());
		}

		/**
		 * Checks to see if the object appears to be valid
		 * 
		 * @return true if this appears to be a valid object
		 * @todo Not functional
		 * 
		 * @since Karma 1.0.0
		 */
		bool IsValidLowLevel() const;

		/** 
		 * Helper function that sets the appropriate flag based on PK being enabled or not
		 *
		 * @todo Need to write decent garbage collection system. Then this may become useful.
		 * @since Karma 1.0.0
		 */
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
		/** 
		 * Returns true if this object is of the specified type.
		 *
		 * @param SomeBase	The UClass that this UObject is subjected to the comparison
		 * @see UObjectBase::GetTypedOuter
		 *
		 * @since Karma 1.0.0
		 */
		template <typename OtherClassType>
		FORCEINLINE bool IsA(OtherClassType SomeBase) const
		{
			// Atm we don't have the cyclic dependency therefore we have forward declared
			// UClass. Maybe we will have some dependency in future, then
			// when we have a cyclic dependency between UObjectBase and UClass,
			// we use a template to allow inlining of something we haven't yet seen, because it delays compilation until the function is called.

			// 'static_assert' that this thing is actually a UClass pointer or convertible to it.
			const UClass* SomeBaseClass = SomeBase;

			KR_CORE_ASSERT(SomeBaseClass, "IsA(NULL) cannot yield meaningful results");

			const UClass* ThisClass = GetClass();

			// Stop the compiler doing some unnecessary branching for nullptr checks
			// Would be interesting to write the analog of this
			KR_CORE_ASSERT(ThisClass, "Couldn't find UClass of {0}. Comparing with null is useless", this->GetName());

			return IsChildOfWorkaround(ThisClass, SomeBaseClass);
		}

		/** 
		 * Returns the UClass that defines the fields of this object
		 *
		 * @since Karma 1.0.0
		 */
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
		 * @since Karma 1.0.0
		 */
		FORCEINLINE EObjectFlags GetFlags() const
		{
			KR_CORE_ASSERT((m_ObjectFlags & ~RF_AllFlags) == 0, "{0} flagged as RF_ALLFlags", GetName());
			return m_ObjectFlags;
		}

		/**
		 * Modifies object flags for a specific object
		 *
		 * @param NewFlags	The EObjectFlags to be set for this object
		 * @see UObjectBase::SetFlagsTo
		 *
		 * @since Karma 1.0.0
		 */
		FORCEINLINE void SetFlags(EObjectFlags NewFlags)
		{
			KR_CORE_ASSERT(!(NewFlags & (RF_MarkAsNative | RF_MarkAsRootSet | RF_PendingKill | RF_Garbage)), "These flags can't be used outside of constructors / internal code");
			KR_CORE_ASSERT(!(NewFlags & (EObjectFlags)(RF_PendingKill | RF_Garbage)) || (GetFlags() & (NewFlags & (EObjectFlags)(RF_PendingKill | RF_Garbage))) == (NewFlags & (EObjectFlags)(RF_PendingKill | RF_Garbage)), "RF_PendingKill and RF_garbage can not be set through SetFlags function. Use MarkAsGarbage() instead");

			SetFlagsTo(EObjectFlags (GetFlags() | NewFlags));
		}

		/**
		 * Clears subset of flags for a specific object
		 *
		 * @param FlagsToClear	EObjectFlags to be cleared
		 * @since Karma 1.0.0
		 */
		FORCEINLINE void ClearFlags(EObjectFlags FlagsToClear)
		{
			KR_CORE_ASSERT(!(FlagsToClear & (RF_MarkAsNative | RF_MarkAsRootSet | RF_PendingKill | RF_Garbage)) || FlagsToClear == RF_AllFlags, "These flags can't be used outside of constructors / internal code");
			KR_CORE_ASSERT(!(FlagsToClear & (EObjectFlags)(RF_PendingKill | RF_Garbage)) || (GetFlags() & (FlagsToClear & (EObjectFlags)(RF_PendingKill | RF_Garbage))) == RF_NoFlags, "RF_PendingKill and RF_garbage can not be cleared through ClearFlags function. Use ClearGarbage() instead");

			SetFlagsTo(EObjectFlags (GetFlags() & ~FlagsToClear));
		}

		/**
		 * Used to safely check whether any of the passed in flags are set.
		 *
		 * @param FlagsToCheck	Object flags to check for
		 * @return				true if any of the passed in flags are set, false otherwise  (including no flags passed in)
		 *
		 * @since Karma 1.0.0
		 */
		FORCEINLINE bool HasAnyFlags(EObjectFlags FlagsToCheck) const
		{
			KR_CORE_ASSERT(!(FlagsToCheck & (RF_MarkAsNative | RF_MarkAsRootSet)) || FlagsToCheck == RF_AllFlags, "Illegal flags being used"); // These flags can't be used outside of constructors / internal code
			return (GetFlags() & FlagsToCheck) != 0;
		}

		/**
		 * Used to safely check whether any of the passed in internal flags are set.
		 *
		 * @param FlagsToCheck	Object flags to check for
		 * @return				true if any of the passed in flags are set, false otherwise  (including no flags passed in)
		 *
		 * @since Karma 1.0.0
		 */
		FORCEINLINE bool HasAnyInternalFlags(EInternalObjectFlags FlagsToCheck) const
		{
			return GUObjectStore.IndexToObject(m_InternalIndex)->HasAnyFlags(FlagsToCheck);
		}

		/**
		 * Sets internal flags of the FUObjectItem
		 *
		 * @param FlagsToSet
		 * @since Karma 1.0.0
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
		 *
		 * @todo To be implemented when garbage collection system is finished
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
		 *
		 * @param ObjClass		The UObject object whose class is to be seen
		 * @param TestClass		The UClass object which is to be compared with
		 *
		 * @since Karma 1.0.0
		 */
		static bool IsChildOfWorkaround(const UClass* ObjClass, const UClass* TestClass);
	};
}
