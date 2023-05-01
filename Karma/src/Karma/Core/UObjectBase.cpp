#include "UObjectBase.h"
#include "Core/Object.h"
#include "Core/Class.h"
#include "Package.h"

namespace Karma
{
	UObjectBase::UObjectBase()
	{
		m_ObjectFlags = RF_NoFlags;
		m_InternalIndex = INDEX_NONE;
		m_ClassPrivate = nullptr;
		m_OuterPrivate = nullptr;
		m_NamePrivate = "NoName";

		// Not adding to GUObjectStore
	}

	UObjectBase::UObjectBase(UClass* inClass, EObjectFlags inFlags, EInternalObjectFlags inInternalFlags, UObject* inOuter, const std::string& inName)
		: m_ObjectFlags(inFlags)
		, m_InternalIndex(INDEX_NONE)
		, m_ClassPrivate(inClass)
		, m_OuterPrivate(inOuter)
	{
		KR_CORE_ASSERT(m_ClassPrivate, "Owner class is null");

		// Add to global table.
		AddObject(inName, inInternalFlags);

		// For testing only
		KR_CORE_INFO("UObjectBase Constructor");
	}

	/**
	 * Walks up the list of outers until it finds a package directly associated with the object.
	 *
	 * @return the package the object is in.
	 */
	UPackage* UObjectBase::GetPackage() const
	{
		const UObject* Top = static_cast<const UObject*>(this);
		for (;;)
		{
			// GetExternalPackage will return itself if called on a UPackage
			if (UPackage* Package = Top->GetExternalPackage())
			{
				return Package;
			}
			Top = Top->GetOuter();
		}
	}

	UPackage* UObjectBase::GetExternalPackage() const
	{
		// if we have no outer, consider this a package, packages returns themselves as their external package
		if (m_OuterPrivate == nullptr)
		{
			return static_cast<UPackage*>((UObject*)(this));
		}

		UPackage* ExternalPackage = nullptr;
		if ((GetFlags() & RF_HasExternalPackage) != 0)
		{
			// Maybe we can get away without this path
			// ExternalPackage = GetObjectExternalPackageThreadSafe(this);
			// if the flag is set there should be an override set.
			KR_CORE_ASSERT(ExternalPackage, "External package is null");
		}
		return ExternalPackage;
	}

	/**
	 * Add a newly created object to the name hash tables (for now no hashtables for Karma) and the object array
	 *
	 * @param Name name to assign to this uobject
	 */
	void UObjectBase::AddObject(const std::string& inName, EInternalObjectFlags inSetInternalFlags)
	{
		m_NamePrivate = inName;
		EInternalObjectFlags InternalFlagsToSet = inSetInternalFlags;
		
		UObject* thisElement = static_cast<UObject*>(this);

		KR_CORE_ASSERT(thisElement, "Can't register not UObject types");
		
		GUObjectStore.push_back(thisElement);
		m_InternalIndex = (int32_t) GUObjectStore.size();

		KR_CORE_ASSERT(inName != "", "UObject name can't be empty string");
		KR_CORE_ASSERT(m_InternalIndex >= 0, "m_InternalIndex has to be non-negative");
		
		/*
		if (InternalFlagsToSet != EInternalObjectFlags::None)
		{
			GUObjectArray.IndexToObject(InternalIndex)->SetFlags(InternalFlagsToSet);
		}
		HashObject(this);
		*/

		KR_CORE_ASSERT(IsValidLowLevel(), "Not valid UObject from low level perspective");
	}

	/**
	 * Checks to see if the object appears to be valid
	 * @return true if this appears to be a valid object
	 */
	bool UObjectBase::IsValidLowLevel() const
	{
		if (this == nullptr)
		{
			KR_CORE_WARN("NULL object");
			return false;
		}
		if (!m_ClassPrivate)
		{
			KR_CORE_WARN("Object is not registered");
			return false;
		}

		return true;// GUObjectArray.IsValid(this);
	}

	void UObjectBase::MarkAsGarbage()
	{
	}

	void UObjectBase::ClearGarbage()
	{
	}

	/**
	 * Traverses the outer chain searching for the next object of a certain type.  (T must be derived from UObject)
	 *
	 * @param	Target class to search for
	 * @return	a pointer to the first object in this object's Outer chain which is of the correct type.
	 */
	UObject* UObjectBase::GetTypedOuter(UClass* Target) const
	{
		UObject* Result = nullptr;
		for (UObject* NextOuter = m_OuterPrivate; Result == nullptr && NextOuter != nullptr; NextOuter = NextOuter->GetOuter())
		{
			if (NextOuter->IsA(Target))
			{
				Result = NextOuter;
			}
		}
		return Result;
	}

	bool UObjectBase::IsChildOfWorkaround(const UClass* ObjClass, const UClass* TestCls)
	{
		return ObjClass->IsChildOf(TestCls);
	}
}
