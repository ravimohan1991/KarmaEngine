#pragma once

#include "krpch.h"

#include "Object.h"

namespace Karma
{
	class UClass;
	//class UStruct;

	/*-----------------------------------------------------------------------------
		UField.
	-----------------------------------------------------------------------------*/

	//
	// Base class of reflection data objects.
	//
	class KARMA_API UField : public UObject
	{
		/** Next Field in the linked list */
		UField* m_Next;

		/** Goes up the outer chain to look for a UClass */
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
	public:
		/** Returns true if this struct either is SomeBase, or is a child of SomeBase. This will not crash on null structs */
		bool IsChildOf(const UStruct* SomeBase) const;

		/** Struct this inherits from, may be null */
		UStruct* GetSuperStruct() const
		{
			return m_SuperStruct;
		}

	private:
		/** Struct this inherits from, may be null */
		UStruct* m_SuperStruct;
	};

	/**
	 * An object class.
	 */
	class KARMA_API UClass : public UStruct
	{
	public:
		/** Constructor */
		UClass(const std::string& name);

		/** The required type for the outer of instances of this class */
		UClass* ClassWithin;

		/** Jugaad */
		std::string m_NamePrivate;

	public:
		// UObject interface.
		virtual const std::string& GetDesc() override;
	};
}