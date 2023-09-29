/**
 * @file Class.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the class UField, UStruct, and UClass.
 * @version 1.0
 * @date March 2, 2023
 *
 * @copyright Karma Engine copyright(c) People of India
 */

#pragma once

#include "Object.h"

namespace Karma
{
	/*-----------------------------------------------------------------------------
		UField.
	-----------------------------------------------------------------------------*/

	/**
	 * @brief Base class of reflection data objects.
	 */
	class KARMA_API UField : public UObject
	{
		DECLARE_KARMA_CLASS(UField, UObject)

		/** Next Field in the linked list */
		//UField* m_Next;

		/**
		 * @brief Goes up the outer chain to look for a UClass.
		 *
		 * Basically looks for the  m_OuterPrivate  in the chinese chain
		 * of outer objects.
		 *
		 * @return the outer or m_OuterPrivate from the chinese chain member
		 */
		UClass* GetOwnerClass() const;

		/** Goes up the outer chain to look for a UStruct */
		//UStruct* GetOwnerStruct() const;

		/**
		 * Returns a human readable string that was assigned to this field at creation.
		 * By default this is the same as GetName() but it can be overridden if that is an internal-only name.
		 * This name is consistent in editor/cooked builds, is not localized, and is useful for data import/export.
		 */
		const std::string& GetAuthoredName() const;
	};

	/*-----------------------------------------------------------------------------
		UStruct.
	-----------------------------------------------------------------------------*/

	/**
	 * @brief Base class for all UObject types that contain fields.
	 */
	class KARMA_API UStruct : public UField
	{
		DECLARE_KARMA_CLASS(UStruct, UField)

	public:
		/** 
		 * Returns true if this struct either is SomeBase, or is a child of SomeBase. This will not crash on null structs
		 *
		 * @return true if the relation is found, else false
		 * @todo Need to write a decent class registration code for class comparison
		 *
		 * @since Karma 1.0.0
		 */
		bool IsChildOf(const UStruct* SomeBase) const;

		/** 
		 * Struct this inherits from, may be null
		 *
		 * @remark Gets set by the routine GetPrivateStaticClassBody()
		 * @see InitializePrivateStaticClass()
		 *
		 * @since Karma 1.0.0
		 */
		UStruct* GetSuperStruct() const
		{
			return m_SuperStruct;
		}

		/** 
		 * @brief Total size of all UProperties, the allocated structure may be larger due to alignment
		 *
		 * The size, in bytes, of the UObject object whose UStruct (rather UClass) is this object.
		 *
		 * @remark Usually the size is of the order of few 100 bytes
		 * @see StaticAllocateObject() in UObjectGlobals.cpp for usage of this routine for allocating UObject with this size
		 * @see DECLARE_KARMA_CLASS(TClass, TSuperClass) in GFrameworkMacros.h for evaluation of the property size (sizeof(TClass))
		 *
		 *	~~~~~~~~~~~~~~~~~~~~~{.c}
		 *	GetPrivateStaticClassBody( \
		 *	    "GeneralPackage", \
		 *	    #TClass, \
		 *	    returnClass_##TClass, \
		 *	    sizeof(TClass), \
		 *	    alignof(TClass), \
		 *	    (ClassConstructorType)InternalConstructor<TClass>, \
		 *	    &TClass::Super::StaticClass \
		 *  );
		 *	~~~~~~~~~~~~~~~~~~~~~
		 *	@since Karma 1.0.0
		 */
		FORCEINLINE size_t GetPropertiesSize() const
		{
			return m_PropertiesSize;
		}

		/**
		 * Sets the super struct pointer.
		 *
		 * @see InitializePrivateStaticClass() in Object.cpp
		 *
		 * @note that this is not sufficient to actually reparent a struct, it simply sets a pointer.
		 * @since Karma 1.0.0
		 */
		virtual void SetSuperStruct(UStruct* NewSuperStruct);

	private:
		/** Struct this inherits from, may be null */
		UStruct* m_SuperStruct;

	public:
		/** 
		 * Total size of all UProperties, the allocated structure may be larger due to alignment 
		 * 
		 * From Epic's code, seems like the size of this object is m_ProperitesSize, for details see 
		 * https://github.com/EpicGames/UnrealEngine/blob/5ca9da84c694c6eee288c30a547fcaa1a40aed9b/Engine/Source/Runtime/CoreUObject/Public/UObject/ObjectMacros.h#L2016
		 * 
		 * Ponder why in the world this field is public.
		 *
		 * @since Karma 1.0.0
		 */
		size_t m_PropertiesSize;

		/** 
		 * Alignment of structure in memory, structure will be at least this large
		 *
		 * @remark Usually UObjects are 16 bytes aligned
		 * @see FUObjectAllocator::AllocateUObject
		 * @note This variable is public. Ponder why.
		 * @since Karma 1.0.0
		 */
		size_t m_MinAlignment;
	};

	/**
	 * @brief An object class.
	 *
	 * An UObject derived object with class identification including size, description, class flags and all that
	 */
	class KARMA_API UClass : public UStruct
	{
		DECLARE_KARMA_CLASS(UClass, UStruct)

	public:
		/** 
		 * @brief Default constructor
		 *
		 * @since Karma 1.0.0
		 */
		UClass();

		/**
		 * @brief Overloaded constructor
		 *
		 * @param name 				Set the m_NamePrivate of the UClass UObject that is class name
		 * 							of the UObject of UClass
		 * @since Karma 1.0.0
		 */
		UClass(const std::string& name);

		/**
		 * A useful constructor for StaticClass initialization
		 *
		 * @param name					Set the m_NamePrivate of the UClass UObject that is class name
		 * 								of the UObject of UClass
		 * @param size					The size (in bytes) of the UObject this object is UClass of
		 * @param alignment				Alignment of UObject, this object is UClass of, in memory. The UObject will be at least this large
		 * @param inClassConstructor	The custom constructor (InternalConstructor<TClass>) to be called after UObject allocation and
		 * 								during UObject initialization. Calls <b>placement new</b> with specified UObject class constructor.
		 *								See <b>InClass->m_ClassConstructor</b> line in the routine StaticConstructObject_Internal(const FStaticConstructObjectParameters&)
		 * @see GetPrivateStaticClassBody in Object.cpp
		 * @since Karma 1.0.0
		 */
		UClass(const std::string& name, size_t size, size_t alignment, ClassConstructorType inClassConstructor);

		// The required type for the outer of instances of this class
		//UClass* m_ClassWithin;

		// Only for hot reload purposes
		// typedef UObject*	(*ClassVTableHelperCtorCallerType)	(FVTableHelper& Helper);

		/**
		 * A custom constructor for the UObject object this class is UClass of
		 *
		 * @remark This member is public because we explicitly call this member after UObjectAllocation in StaticConstructObject_Internal(const FStaticConstructObjectParameters&)
		 * @since Karma 1.0.0
		 */
		ClassConstructorType m_ClassConstructor;

		/** 
		 * Class flags; See Karma::EClassFlags for more information
		 *
		 * @since Karma 1.0.0
		 */
		EClassFlags m_ClassFlags;

	public:
		// UObject interface.
		/**
		 * Fetch a human readable description for the UClass
		 *
		 * @remark	Gets the m_NamePrivate of the UClass UObject which is the name of the UClass
		 * 			of a UObject
		 * @since Karma 1.0.0
		 */
		virtual const std::string& GetDesc() override;

		/**
		 * Used to safely check whether the passed in flag is set.
		 *
		 * @param	FlagsToCheck		Class flag(s) to check for
		 *
		 * @return	true if the passed in flag is set, false otherwise
		 *			(including no flag passed in, unless the FlagsToCheck is CLASS_AllFlags)
		 * @since Karma 1.0.0
		 */
		FORCEINLINE bool HasAnyClassFlags(EClassFlags FlagsToCheck) const
		{
			return EnumHasAnyFlags(m_ClassFlags, FlagsToCheck) != 0;
		}

	public:
		/**
		 * Allows class to provide data to the object initializer that can affect how native class subobjects are created.
		 *
		 * @todo Placeholder for the moment.
		 * @since Karma 1.0.0
		 */
		virtual void SetupObjectInitializer(FObjectInitializer& ObjectInitializer) const {}

		/**
		 * Get the default object from the class
		 * @param bCreateIfNeeded	If true (default) then the CDO is created if it is null
		 * @return					the CDO for this class
		 * @todo Will be completely written once the scope of defaultobject is needed
		 * @since Karma 1.0.0
		 */
		UObject* GetDefaultObject(bool bCreateIfNeeded = true) const
		{
			if (/*m_ClassDefaultObject == nullptr &&*/ bCreateIfNeeded)
			{
				// some UE macro
				const_cast<UClass*>(this)->CreateDefaultObject();
			}

			return nullptr;//m_ClassDefaultObject;
		}

		/**
		 * Get the default object from the class and cast to a particular type
		 *
		 * @return		the CDO for this class
		 * @since Karma 1.0.0
		 */
		template<class T>
		T* GetDefaultObject() const
		{
			UObject* Ret = GetDefaultObject();
			KR_CORE_ASSERT(Ret->IsA(T::StaticClass()), "Class {0} is not subclass of UObject", T::GetName());
			return (T*)Ret;
		}

		/** 
		 * Returns parent class, the parent of a Class is always another class
		 *
		 * @since Karma 1.0.0
		 */
		UClass* GetSuperClass() const
		{
			return (UClass*)GetSuperStruct();
		}

		/** 
		 * Alignment of structure in memory, structure will be at least this large
		 *
		 * @since Karma 1.0.0
		 */
		FORCEINLINE size_t GetMinAlignment() const
		{
			return m_MinAlignment;
		}

		// UStruct interface.

		/**
		 * Sets the super struct pointer.
		 *
		 * @see InitializePrivateStaticClass() in Object.cpp
		 *
		 * @note that this is not sufficient to actually reparent a struct, it simply sets a pointer.
		 * @since Karma 1.0.0
		 */
		virtual void SetSuperStruct(UStruct* NewSuperStruct) override;

		/**
		 * Helper method to assist with initializing object properties from an explicit list.
		 *
		 * @param	InStruct			the current scope for which the given property list applies
		 * @param	DataPtr				destination address (where to start copying values to)
		 * @param	DefaultDataPtr		source address (where to start copying the defaults data from)
		 * @since Karma 1.0.0
		 */
		virtual void InitPropertiesFromCustomList(uint8_t* DataPtr, const uint8_t* DefaultDataPtr) {}

	protected:
		/**
		 * Get the default object from the class, creating it if missing, if requested or under a few other circumstances
		 * @return		the CDO for this class
		 *
		 * @since Karma 1.0.0
		 */
		virtual UObject* CreateDefaultObject();

	public:
		/** The class default object; used for delta serialization and object initialization */
		//UObject* m_ClassDefaultObject;

	private:
		/** Used to check if the class layout is currently changing and therefore is not ready for a CDO to be created */
		//bool m_bLayoutChanging;
	};
}
