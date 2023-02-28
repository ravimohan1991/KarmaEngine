#include "UObjectGlobals.h"
#include "Object.h"

namespace Karma
{

	UObject* StaticConstructObject_Internal(const FStaticConstructObjectParameters& Params)
	{
		//const UClass* InClass = Params.Class;
		UObject* InOuter = Params.m_Outer;
		const std::string& InName = Params.m_Name;
		EObjectFlags InFlags = Params.m_SetFlags;
		UObject* InTemplate = Params.m_Template;

		//LLM_SCOPE(ELLMTag::UObject);

		//SCOPE_CYCLE_COUNTER(STAT_ConstructObject);
		UObject* Result = NULL;

		//checkf(!InTemplate || InTemplate->IsA(InClass) || (InFlags & RF_ClassDefaultObject), TEXT("StaticConstructObject %s is not an instance of class %s and it is not a CDO."), *GetFullNameSafe(InTemplate), *GetFullNameSafe(InClass)); // template must be an instance of the class we are creating, except CDOs

		// Subobjects are always created in the constructor, no need to re-create them unless their archetype != CDO or they're blueprint generated.
		// If the existing subobject is to be re-used it can't have BeginDestroy called on it so we need to pass this information to StaticAllocateObject.	
		// const bool bIsNativeClass = InClass->HasAnyClassFlags(CLASS_Native | CLASS_Intrinsic);
		// const bool bIsNativeFromCDO = bIsNativeClass &&
			//(
			//	!InTemplate ||
			//	(InName != NAME_None && (Params.bAssumeTemplateIsArchetype || InTemplate == UObject::GetArchetypeFromRequiredInfo(InClass, InOuter, InName, InFlags)))
			//	);

		// Do not recycle subobjects when performing hot-reload as they may contain old property values.
		//const bool bCanRecycleSubobjects = bIsNativeFromCDO && (!(InFlags & RF_DefaultSubObject) || !FUObjectThreadContext::Get().IsInConstructor) && !IsReloadActive();

		bool bRecycledSubobject = false;
		Result = StaticAllocateObject(InOuter, InName, InFlags, Params.m_InternalSetFlags);

		KR_CORE_ASSERT(Result != nullptr, "Couldn't create new object.");
		/*
		if (GIsEditor && GUndo &&
			(InFlags & RF_Transactional) && !(InFlags & RF_NeedLoad) &&
			!InClass->IsChildOf(UField::StaticClass()) &&
			// Do not consider object creation in transaction if the object is marked as async or in being async loaded 
			!Result->HasAnyInternalFlags(EInternalObjectFlags::Async | EInternalObjectFlags::AsyncLoading))
		{
			// Set RF_PendingKill and update the undo buffer so an undo operation will set RF_PendingKill on the newly constructed object.
			Result->MarkAsGarbage();
			SaveToTransactionBuffer(Result, false);
			Result->ClearGarbage();
		}*/

		return Result;
	}

	UObject* StaticAllocateObject(UObject* inOuter, const std::string& inName, EObjectFlags inFlags,
		EInternalObjectFlags internalSetFlags)
	{
		// Not this simple
		return new UObject();
	}
}