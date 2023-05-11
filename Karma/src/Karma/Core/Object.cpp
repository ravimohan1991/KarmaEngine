#include "Object.h"
#include "Class.h"
#include "World.h"
#include "UObjectAllocator.h"
#include "KarmaMemory.h"

namespace Karma
{
	UObject::UObject() : UObjectBase()
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
		uint32_t InSize,
		uint32_t InAlignment,
		/*EClassFlags InClassFlags,
		EClassCastFlags InClassCastFlags,
		const TCHAR* InConfigName,
		UClass::ClassConstructorType InClassConstructor,
		UClass::ClassVTableHelperCtorCallerType InClassVTableHelperCtorCaller,
		FUObjectCppClassStaticFunctions&& InCppClassStaticFunctions,*/
		StaticClassFunctionType InSuperClassFn
	/*UClass::StaticClassFunctionType InWithinClassFn*/)
	{
		// TODO: search if already exists
		//size_t ClassSize = sizeof(UClass);

		ReturnClass = (UClass*)GUObjectAllocator.AllocateUObject(InSize, InAlignment, true);

		// Note: UE doesn't 0 initialize
		FMemory::Memzero((void*)ReturnClass, InSize);

		// call the constructor
		ReturnClass = new (ReturnClass) UClass(Name, InSize, InAlignment);

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

		/* No recursive ::StaticClass calls allowed. Setup extras. */
		if (TClass_Super_StaticClass != TClass_PrivateStaticClass)
		{
			TClass_PrivateStaticClass->SetSuperStruct(TClass_Super_StaticClass);
		}
		else
		{
			TClass_PrivateStaticClass->SetSuperStruct(nullptr);
		}

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
}