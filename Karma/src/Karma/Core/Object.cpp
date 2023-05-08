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

	UClass* UObject::StaticClass(UObject* someObject)
	{
		UClass* StaticUClass;
		size_t ClassSize = sizeof(UClass);

		StaticUClass = (UClass*)GUObjectAllocator.AllocateUObject(ClassSize, alignof(UClass), true);

		// Note: UE doesn't 0 initialize
		FMemory::Memzero((void*)StaticUClass, ClassSize);

		// call the constructor
		StaticUClass = new (StaticUClass) UClass(typeid(*someObject).name());

		return StaticUClass;
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
}