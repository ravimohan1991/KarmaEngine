#include "UObjectBase.h"
#include "Core/Object.h"
#include "Core/Class.h"
#include "Package.h"

namespace Karma
{
	// Console variable so that GarbageCollectorSettings work in the editor but we don't want to use it in runtime
	// as we can't support changing its value from console
	int32 GPendingKillEnabled = 1;

	// If GPendingKillEnabled is true, objects marked as PendingKill will be automatically nulled and destroyed by Garbage Collector.
	bool UObjectBase::m_bPendingKillDisabled = !GPendingKillEnabled;

	UObjectBase::UObjectBase()
	{
	}

	// This constructor is called by StaticAllocateObject
	UObjectBase::UObjectBase(UClass* inClass, EObjectFlags inFlags, EInternalObjectFlags inInternalFlags, UObject* inOuter, const std::string& inName)
		: m_ObjectFlags(inFlags)
		, m_InternalIndex(INDEX_NONE)
		, m_ClassPrivate(inClass)
		, m_OuterPrivate(inOuter)
	{
		KR_CORE_ASSERT(m_ClassPrivate, "Owner class is null");

		// Add to global table.
		AddObject(inName, inInternalFlags);
	}

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

	void UObjectBase::AddObject(const std::string& inName, EInternalObjectFlags inSetInternalFlags)
	{
		m_NamePrivate = inName;
		EInternalObjectFlags InternalFlagsToSet = inSetInternalFlags;

		UObject* anObject = static_cast<UObject*>(this);
		KR_CORE_ASSERT(anObject, "Can't and won't add non UObject types in the store");

		GUObjectStore.AddUObject(anObject);

		KR_CORE_ASSERT(inName != "", "UObject name can't be empty string");
		KR_CORE_ASSERT(m_InternalIndex >= 0, "m_InternalIndex has to be non-negative");

		/*
		if (InternalFlagsToSet != EInternalObjectFlags::None)
		{
			GUObjectArray.IndexToObject(InternalIndex)->SetFlags(InternalFlagsToSet);
		}
		HashObject(this);
		*/

		CacheObject(anObject);// We are using this instead of HashObject because of simplicity

		KR_CORE_ASSERT(IsValidLowLevel(), "Not valid UObject from low level perspective");
	}

	bool UObjectBase::IsUnreachable() const
	{
		//return GUObjectStore.IndexToObject(m_InternalIndex)->IsUnreachable();

		// is this the right way? See UObjectGlobals::StaticAllocateObject in ue, Obj->SetInternalFlags()
		// intersetflags are different from m_ObjectFlags
		return !!(m_ObjectFlags & int32_t(EInternalObjectFlags::Unreachable));
	}

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

		return GUObjectStore.IsValid(this);
	}

	void UObjectBase::MarkAsGarbage()
	{
	}

	void UObjectBase::ClearGarbage()
	{
	}

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

	bool UObjectBase::IsChildOfWorkaround(const UClass* ObjClass, const UClass* TestClass)
	{
		return ObjClass->IsChildOf(TestClass);
	}
}
