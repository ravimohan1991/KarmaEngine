#pragma once

#include "krpch.h"

#include "Object.h"

namespace Karma
{

	/*-----------------------------------------------------------------------------
		UField.
	-----------------------------------------------------------------------------*/

	//
	// Base class of reflection data objects.
	//
	class KARMA_API UField : public UObject
	{
		DECLARE_KARMA_CLASS(UField, UObject)

		/** Next Field in the linked list */
		UField* m_Next;

		/**
		 * Goes up the outer chain to look for a UClass.
		 * Basically looks for the  m_OuterPrivate  in the chinese chain
		 * of outer objects.
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
	 * Base class for all UObject types that contain fields.
	 */
	class KARMA_API UStruct : public UField
	{
		DECLARE_KARMA_CLASS(UStruct, UField)

	public:
		/** Returns true if this struct either is SomeBase, or is a child of SomeBase. This will not crash on null structs */
		bool IsChildOf(const UStruct* SomeBase) const;

		/** Struct this inherits from, may be null */
		UStruct* GetSuperStruct() const
		{
			return m_SuperStruct;
		}

		/** Total size of all UProperties, the allocated structure may be larger due to alignment */
		FORCEINLINE size_t GetPropertiesSize() const
		{
			return m_PropertiesSize;
		}

		/**
		 * Sets the super struct pointer and updates hash information as necessary.
		 * Note that this is not sufficient to actually reparent a struct, it simply sets a pointer.
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
		 */
		size_t m_PropertiesSize;

		/** Alignment of structure in memory, structure will be at least this large */
		// Public?
		size_t m_MinAlignment;
	};

	/**
	 * An object class.
	 */
	class KARMA_API UClass : public UStruct
	{
		DECLARE_KARMA_CLASS(UClass, UStruct)

	public:
		/** Constructors */
		UClass();
		UClass(const std::string& name);

		/**
		 * A useful constructor for StaticClass initialization
		 * 
		 * @see GetPrivateStaticClassBody in Object.cpp
		 */
		UClass(const std::string& name, size_t size, size_t alignment);

		/** The required type for the outer of instances of this class */
		//UClass* m_ClassWithin;

	public:
		// UObject interface.
		virtual const std::string& GetDesc() override;

	public:
		/**
		 * Get the default object from the class
		 * @param	bCreateIfNeeded if true (default) then the CDO is created if it is null
		 * @return		the CDO for this class
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
		 * @return		the CDO for this class
		 */
		template<class T>
		T* GetDefaultObject() const
		{
			UObject* Ret = GetDefaultObject();
			KR_CORE_ASSERT(Ret->IsA(T::StaticClass()), "Class {0} is not subclass of UObject");
			return (T*)Ret;
		}

		/** Returns parent class, the parent of a Class is always another class */
		UClass* GetSuperClass() const
		{
			return (UClass*)GetSuperStruct();
		}

		/** Alignment of structure in memory, structure will be at least this large */
		FORCEINLINE size_t GetMinAlignment() const
		{
			return m_MinAlignment;
		}

		// UStruct interface.
		virtual void SetSuperStruct(UStruct* NewSuperStruct) override;

	protected:
		/**
		 * Get the default object from the class, creating it if missing, if requested or under a few other circumstances
		 * @return		the CDO for this class
		 **/
		virtual UObject* CreateDefaultObject();

	public:
		/** The class default object; used for delta serialization and object initialization */
		//UObject* m_ClassDefaultObject;

	private:
		/** Used to check if the class layout is currently changing and therefore is not ready for a CDO to be created */
		//bool m_bLayoutChanging;
	};
}
