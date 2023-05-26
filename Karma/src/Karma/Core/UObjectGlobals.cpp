#include "UObjectGlobals.h"
#include "Object.h"
#include "Core/Class.h"
#include "Ganit/KarmaMath.h"
#include "UObjectAllocator.h"
#include "Karma/Core/TrueCore/KarmaMemory.h"
#include "Karma/Core/Package.h"

namespace Karma
{
	std::vector<UObject*> GUObjectStore;

	/** Whether we are still in the initial loading proces. (Got from CoreGlobals.cpp) */
	KARMA_API bool			GIsInitialLoad = true;

	// Try putting in .h also
	//extern FUObjectAllocator GUObjectAllocator;

	UObject* StaticFindObjectFastInternal(const UClass* ObjectClass, const UObject* ObjectPackage, const std::string& ObjectName, bool bExactClass, EObjectFlags ExcludeFlags, EInternalObjectFlags ExclusiveInternalFlags)
	{
		UObject* result = nullptr;

		if (ObjectPackage != nullptr)
		{
			// We start with the primitive type of vector iterations
			// how about const referencing element
			for (auto& element : GUObjectStore)
			{
				if (element->GetName() == ObjectName

					/* Don't return objects that have any of the exclusive flags set */
					&& !element->HasAnyFlags(ExcludeFlags)

					/* check that the object has the correct Outer */
					&& element->GetOuter() == ObjectPackage

					/** If a class was specified, check that the object is of the correct class (hierarchy) */
					&& (ObjectClass == nullptr || (bExactClass ? element->GetClass() == ObjectClass : element->IsA(ObjectClass)))

					/** Include (or not) pending kill objects */
					// leaving for now. may become relevant later
					)
				{
					if (result)
					{
						KR_CORE_WARN("Ambigous search, could be {0} or {1}", result->GetName(), element->GetName());
					}
					else
					{
						result = element;
					}
				}
			}

			// if the search fail and the OuterPackage is a UPackage, lookup potential external package
			// for now we are not concerned with UPackage
		}
		else
		{
			// ThreadHash.Hash.Find(ObjectHash) and whatnot
			// bAnyPackage should be set false here
		}

		return result;
	}

	UObject* StaticConstructObject_Internal(const FStaticConstructObjectParameters& Params)
	{
		const UClass* InClass = Params.m_Class;
		UObject* InOuter = Params.m_Outer;
		const std::string& InName = Params.m_Name;
		EObjectFlags InFlags = Params.m_SetFlags;
		//UObject* InTemplate = Params.m_Template;

		/*if (InTemplate)
		{
			KR_CORE_ASSERT(InTemplate->IsA(InClass) || (InFlags & RF_ClassDefaultObject), "StaticConstructObject {0} is not an instance of class {1} and it is not a CDO", InTemplate->GetName(), InClass->GetName()); // template must be an instance of the class we are creating, except CDOs
		}*/

		// Need to write in garbage collection code

		UObject* Result = nullptr;

		Result = StaticAllocateObject(InClass, InOuter, InName, InFlags, Params.m_InternalSetFlags);

		KR_CORE_ASSERT(Result != nullptr, "Couldn't create new object.");

		return Result;
	}

	// For spawning AActors, inOuter is LevelToSpawnIn
	UObject* StaticAllocateObject(const UClass* inClass, UObject* inOuter, const std::string& inName, EObjectFlags inFlags,
		EInternalObjectFlags internalSetFlags)
	{
		KR_CORE_ASSERT(inOuter != INVALID_OBJECT, "");
		
		// Also need to write (InClass->ClassWithin && InClass->ClassConstructor)
		KR_CORE_ASSERT(inClass != nullptr, "The class of the object is not valid");

		UObject* Object = nullptr;
		std::string objectName = inName;

		if (objectName == "")
		{
			objectName = "NoName";
			KR_CORE_WARN("Attempting to create UObject with empty name string. Defaulting to NoName");
		}

		// see if object already exists in GUObjectStore. Maybe not functional
		Object = StaticFindObjectFastInternal(nullptr, inOuter, objectName, true);

		int32 totalSize = inClass->GetPropertiesSize();

		KR_CORE_ASSERT(totalSize, "Empty data structure");

		if (Object == nullptr)
		{
			int32 Alignment = FMath::Max(4, inClass->GetMinAlignment());

			// This is the line corresponding to the instantiation of UObjects
			// from Karma's memory system known by the name Smriti.
			Object = (UObject*)GUObjectAllocator.AllocateUObject(totalSize, Alignment, GIsInitialLoad);
		}

		FMemory::Memzero((void*)Object, totalSize);

		EObjectFlags relevantFlags = EObjectFlags (inFlags | RF_NeedInitialization);

		// Oddly, UE does a placement new by calling the constructor of UObjectBase, hehe. When I do that
		// I get wierd offsets in the datamembers. A moment for a question.
		new((void*)Object) UObject(const_cast<UClass*>(inClass), relevantFlags, internalSetFlags, inOuter, inName);

		return Object;
	}

	UPackage* CreatePackage(const std::string& PackageName)
	{
		std::string InName;

		if (PackageName != "")
		{
			InName = PackageName;
		}

		/*
		if (InName.Contains(TEXT("//"), ESearchCase::CaseSensitive))
		{
			UE_LOG(LogUObjectGlobals, Fatal, TEXT("Attempted to create a package with name containing double slashes. PackageName: %s"), PackageName);
		}

		if (InName.EndsWith(TEXT("."), ESearchCase::CaseSensitive))
		{
			FString InName2 = InName.Left(InName.Len() - 1);
			UE_LOG(LogUObjectGlobals, Log, TEXT("Invalid Package Name entered - '%s' renamed to '%s'"), *InName, *InName2);
			InName = InName2;
		}

		if (InName.Len() == 0)
		{
			InName = MakeUniqueObjectName(nullptr, UPackage::StaticClass()).ToString();
		}*/

		UObject* Outer = nullptr;
		//ResolveName(Outer, InName, true, false);

		UPackage* Result = nullptr;

		/*
		if (InName.Len() == 0)
		{
			UE_LOG(LogUObjectGlobals, Fatal, TEXT("%s"), TEXT("Attempted to create a package with an empty package name."));
		}*/

		if (InName != "")
		{
			//Result = FindObject<UPackage>(nullptr, *InName);

			if (Result == NULL)
			{
				//FName NewPackageName(*InName, FNAME_Add);
				
				/*if (FPackageName::IsShortPackageName(NewPackageName))
				{
					UE_LOG(LogUObjectGlobals, Warning, TEXT("Attempted to create a package with a short package name: %s Outer: %s"), PackageName, Outer ? *Outer->GetFullName() : TEXT("NullOuter"));
				}
				else
				{*/
					Result = NewObject<UPackage>(nullptr, UPackage::StaticClass(), /*NewPackageName*/InName, RF_Public);
				//}
			}
		}
		else
		{
			//UE_LOG(LogUObjectGlobals, Fatal, TEXT("%s"), TEXT("Attempted to create a package named 'None'"));
			KR_CORE_ASSERT(false, "Attempted to create a package not even named");
		}

		return Result;
	}
}
