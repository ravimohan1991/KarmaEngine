#include "Object.h"
#include "Class.h"
#include "World.h"
#include "UObjectAllocator.h"
#include "KarmaMemory.h"
#include "Package.h"

namespace Karma
{
	UObject::UObject()
	{
	}

	UObject::UObject(UClass* inClass, EObjectFlags inFlags, EInternalObjectFlags inInternalFlags, UObject* inOuter, const std::string& inName) : 
		UObjectBase(inClass, inFlags, inInternalFlags, inOuter, inName)
	{
	}

	void UObject::FinishDestroy()
	{
	}

	class UWorld* UObject::GetWorld() const
	{
		if (UObject* Outer = GetOuter())
		{
			return Outer->GetWorld();
		}

		return nullptr;
	}

	bool UObject::IsValidChecked(const UObject * Test)
	{
		KR_CORE_ASSERT(Test, "Test pointer is null");
		return !Test->HasAnyFlags(EObjectFlags(RF_PendingKill | RF_Garbage));
	}

	void UObject::BeginDestroy()
	{
		// Sanity assertion to ensure ConditionalBeginDestroy is the only code calling us.
		if (!HasAnyFlags(RF_BeginDestroyed))
		{
			KR_CORE_ASSERT(false, "Trying to call UObject::BeginDestroy from outside of UObject::ConditionalBeginDestroy on object {0}. Please fix up the calling code.",
				GetName());
		}

		// Some macro based code

		// Remove from linker's export table.
		/*SetLinker(NULL, INDEX_NONE);

		LowLevelRename(NAME_None);
		// Remove any associated external package, at this point
		SetExternalPackage(nullptr);

		// ensure BeginDestroy has been routed back to UObject::BeginDestroy.
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		DebugBeginDestroyed.RemoveSingle(this);
#endif*/
	}

	void GetPrivateStaticClassBody(
		const std::string& PackageName,
		const std::string& Name,
		UClass*& ReturnClass,
		/*void(*RegisterNativeFunc)(),*/
		size_t InSize,
		size_t InAlignment,
		/*EClassFlags InClassFlags,
		EClassCastFlags InClassCastFlags,
		const TCHAR* InConfigName,*/
		ClassConstructorType InClassConstructor,
		/*UClass::ClassVTableHelperCtorCallerType InClassVTableHelperCtorCaller,
		FUObjectCppClassStaticFunctions&& InCppClassStaticFunctions,*/
		StaticClassFunctionType InSuperClassFn
		/*UClass::StaticClassFunctionType InWithinClassFn*/)
	{
		// search if already exists, deviation from UE
		UClass* result = nullptr;

		for (auto& element : GUObjectStore)
		{
			if (element->m_Object->GetName() == Name)
			{
				if (result)
				{
					KR_CORE_WARN("Ambigous search, could be {0} or {1}", result->GetName(), element->m_Object->GetName());
				}
				else
				{
					result = (UClass*)element->m_Object;
				}
			}
		}

		if(result)
		{
			ReturnClass = result;
			return;
		}

		void* aPtr = reinterpret_cast<void*>(GUObjectAllocator.AllocateUObject(sizeof(UClass), alignof(UClass), true));
		GUObjectAllocator.DumpUObjectsInformation(aPtr, Name, sizeof(UClass), alignof(UClass), nullptr);

		ReturnClass = (UClass*)aPtr;

		// Note: UE doesn't 0 initialize
		FMemory::Memzero((void*)ReturnClass, sizeof(UClass));

		// Call the constructor
		ReturnClass = ::new (ReturnClass) UClass(Name, InSize, InAlignment, InClassConstructor);

		InitializePrivateStaticClass(
			InSuperClassFn(),
			ReturnClass,
			nullptr,
			PackageName,
			Name
		);
	}

	/**
	 * Shared function called from the various InitializePrivateStaticClass functions generated my the IMPLEMENT_CLASS macro.
	 */
	void InitializePrivateStaticClass(
		class UClass* TClass_Super_StaticClass,
		class UClass* TClass_PrivateStaticClass,
		class UClass* TClass_WithinClass_StaticClass,
		const std::string& PackageName,
		const std::string& Name
	)
	{
		//TRACE_LOADTIME_CLASS_INFO(TClass_PrivateStaticClass, Name);

		TClass_PrivateStaticClass->SetSuperStruct(TClass_Super_StaticClass);

		/*
		TClass_PrivateStaticClass->ClassWithin = TClass_WithinClass_StaticClass;

		// Register the class's dependencies, then itself.
		TClass_PrivateStaticClass->RegisterDependencies();
		{
			// Defer
			TClass_PrivateStaticClass->Register(PackageName, Name);
		}
		*/

		// jugaad name is already set by UClass() constructor
	}

#if WITH_EDITOR
	bool UObject::CanModify() const
	{
		return (!HasAnyFlags(RF_NeedInitialization) &&/* !IsGarbageCollecting() && !GExitPurge &&*/ !IsUnreachable());
	}

	bool UObject::Modify(bool bAlwaysMarkDirty/*=true*/)
	{
		bool bSavedToTransactionBuffer = false;

		if (CanModify())
		{
			// Do not consider script packages, as they should never end up in the
			// transaction buffer and we don't want to mark them dirty here either.
			// We do want to consider PIE objects however
			if ((GetPackage()->HasAnyPackageFlags(PKG_ContainsScript | PKG_CompiledIn) == false || GetClass()->HasAnyClassFlags(EClassFlags(CLASS_DefaultConfig | CLASS_Config))) &&
				!HasAnyInternalFlags(EInternalObjectFlags::Async | EInternalObjectFlags::AsyncLoading))
			{
				// Attempt to mark the package dirty and save a copy of the object to the transaction
				// buffer. The save will fail if there isn't a valid transactor, the object isn't
				// transactional, etc.
				//  bSavedToTransactionBuffer = SaveToTransactionBuffer(this, bAlwaysMarkDirty);

				// If we failed to save to the transaction buffer, but the user requested the package
				// marked dirty anyway, do so
				if (!bSavedToTransactionBuffer && bAlwaysMarkDirty)
				{
					//  MarkPackageDirty();
				}
			}
			//FCoreUObjectDelegates::BroadcastOnObjectModified(this);
		}

		return bSavedToTransactionBuffer;
	}
#endif

	bool UObject::IsSelected() const
	{
#if WITH_EDITOR
		return IsSelectedInEditor();
#else
		return false;
#endif
	}
}
