#include "Object.h"
#include "Class.h"
#include "World.h"

namespace Karma
{
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
}