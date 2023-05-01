#include "Object.h"
#include "Class.h"
#include "World.h"

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
		static UClass StaticUClass;

		StaticUClass.SetPName(typeid(*someObject).name());// heh, wanna see how this works
		return &StaticUClass;
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