#include "UObjectGlobals.h"
#include "Object.h"

namespace Karma
{
	// Global UObject array instance
	// All the UObjects created are to be found in this store
	std::vector<UObject> GUObjectStore;

	UObject* StaticConstructObject_Internal(const FStaticConstructObjectParameters& Params)
	{
		//const UClass* InClass = Params.Class;
		UObject* InOuter = Params.m_Outer;
		const std::string& InName = Params.m_Name;
		EObjectFlags InFlags = Params.m_SetFlags;
		UObject* InTemplate = Params.m_Template;

		// Need to write in garbage collection code

		UObject* Result = NULL;

		Result = StaticAllocateObject(InOuter, InName, InFlags, Params.m_InternalSetFlags);

		KR_CORE_ASSERT(Result != nullptr, "Couldn't create new object.");

		return Result;
	}

	UObject* StaticAllocateObject(UObject* inOuter, const std::string& inName, EObjectFlags inFlags,
		EInternalObjectFlags internalSetFlags)
	{
		// Not this simple
		return new UObject();
	}
}