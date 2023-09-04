#include "UObjectGlobals.h"
#include "Object.h"
#include "Core/Class.h"
#include "Ganit/KarmaMath.h"
#include "UObjectAllocator.h"
#include "Karma/Core/TrueCore/KarmaMemory.h"
#include "Karma/Core/Package.h"

namespace Karma
{
	FUObjectArray GUObjectStore;
	KarmaClassObjectMap m_ClassToObjectVectorMap;

	/** Whether we are still in the initial loading proces. (Got from CoreGlobals.cpp) */
	KARMA_API bool			GIsInitialLoad = true;

	// Try putting in .h also
	//extern FUObjectAllocator GUObjectAllocator;

	FObjectInitializer::FObjectInitializer(UObject* InObj, UObject* InObjectArchetype, bool bInCopyTransientsFromClassDefaults, bool 	bInShouldInitializeProps, struct FObjectInstancingGraph* InInstanceGraph)
		: m_Object(InObj)
		, m_ObjectArchetype(InObjectArchetype)
		  // if the SubobjectRoot NULL, then we want to copy the transients from the template, otherwise we are doing a duplicate and we 	want to copy the transients from the class defaults
		, m_bCopyTransientsFromClassDefaults(bInCopyTransientsFromClassDefaults)
		, m_bShouldInitializePropsFromArchetype(bInShouldInitializeProps)
		, m_bSubobjectClassInitializationAllowed(true)
		/*, InstanceGraph(InInstanceGraph)*/
		, m_LastConstructedObject(nullptr)
	{
		//FUObjectThreadContext& ThreadContext = FUObjectThreadContext::Get();
		// Mark we're in the constructor now.
		//ThreadContext.IsInConstructor++;
		//m_LastConstructedObject = ThreadContext.ConstructedObject;
		//ThreadContext.ConstructedObject = Obj;
		//ThreadContext.PushInitializer(this);

		if (m_Object)
		{
			m_Object->GetClass()->SetupObjectInitializer(*this);
		}
	}

	/**
	 * Destructor for internal class to finalize UObject creation (initialize properties) after the real C++ constructor is called.
	 */
	FObjectInitializer::~FObjectInitializer()
	{
		KR_CORE_ASSERT(m_Object != nullptr, "");

		const bool bIsCDO = m_Object->HasAnyFlags(RF_ClassDefaultObject);
		UClass* Class = m_Object->GetClass();

		if (Class != UObject::StaticClass())
		{
			// InClass->GetClass() == NULL when InClass hasn't been fully initialized yet (during static registration)
			if ( !m_ObjectArchetype  && Class->GetClass() )
			{
				m_ObjectArchetype = Class->GetDefaultObject();
			}
		}
		else if (bIsCDO)
		{
			// for the Object CDO, make sure that we do not use an archetype
			KR_CORE_ASSERT(m_ObjectArchetype == nullptr, "");
		}

		PostConstructInit();
	}

	// Binary initialize object properties to zero or defaults.
	void FObjectInitializer::InitProperties(UObject* Obj, UClass* DefaultsClass, UObject* DefaultData, bool 	bCopyTransientsFromClassDefaults)
	{
		KR_CORE_ASSERT(/*!GEventDrivenLoaderEnabled ||*/ !DefaultsClass || !DefaultsClass->HasAnyFlags(RF_NeedLoad), "");
		KR_CORE_ASSERT(/*!GEventDrivenLoaderEnabled ||*/ !DefaultData || !DefaultData->HasAnyFlags(RF_NeedLoad), "");

		//SCOPE_CYCLE_COUNTER(STAT_InitProperties);

		KR_CORE_ASSERT(DefaultsClass && Obj, "");

		UClass* Class = Obj->GetClass();

		// bool to indicate that we need to initialize any non-native properties (native ones were done when the native constructor was 	called by the code that created and passed in a FObjectInitializer object)
		bool bNeedInitialize = !Class->HasAnyClassFlags(EClassFlags(CLASS_Native | CLASS_Intrinsic));


		// bool to indicate that we can use the faster PostConstructLink chain for initialization.
		bool bCanUsePostConstructLink = !bCopyTransientsFromClassDefaults && DefaultsClass == Class;


		if (Obj->HasAnyFlags(RF_NeedLoad))
		{
			bCopyTransientsFromClassDefaults = false;
		}


		if (!bNeedInitialize && bCanUsePostConstructLink)
		{
			// This is just a fast path for the below in the common case that we are not doing a duplicate or initializing a CDO and 	this is all native.
			// We only do it if the DefaultData object is NOT a CDO of the object that's being initialized. CDO data is already 	initialized in the
			// object's constructor.
			if (DefaultData)
			{
				if (Class->GetDefaultObject(false) != DefaultData)
				{
					/*
					for (FProperty* P = Class->PropertyLink; P; P = P->PropertyLinkNext)
					{
						bool bIsTransient = P->HasAnyPropertyFlags(CPF_Transient | CPF_DuplicateTransient | 	CPF_NonPIEDuplicateTransient);
						if (!bIsTransient || !P->ContainsInstancedObjectProperty())
						{
							if (P->IsInContainer(DefaultsClass))
							{
								P->CopyCompleteValue_InContainer(Obj, DefaultData);
							}
						}
					}
					*/
				}
				else
				{
					// Copy all properties that require additional initialization (e.g. CPF_Config).
					/*
					for (FProperty* P = Class->PostConstructLink; P; P = P->PostConstructLinkNext)
					{
						bool bIsTransient = P->HasAnyPropertyFlags(CPF_Transient | CPF_DuplicateTransient | 	CPF_NonPIEDuplicateTransient);
						if (!bIsTransient || !P->ContainsInstancedObjectProperty())
						{
							if (P->IsInContainer(DefaultsClass))
							{
								P->CopyCompleteValue_InContainer(Obj, DefaultData);
							}
						}
					}
					*/
				}
			}
		}
		else
		{
			// As with native classes, we must iterate through all properties (slow path) if default data is pointing at something other 	than the CDO.
			bCanUsePostConstructLink &= (DefaultData == Class->GetDefaultObject(false));


			UObject* ClassDefaults = bCopyTransientsFromClassDefaults ? DefaultsClass->GetDefaultObject() : NULL;
			KR_CORE_ASSERT(/*!GEventDrivenLoaderEnabled ||*/ !bCopyTransientsFromClassDefaults || 	!DefaultsClass->GetDefaultObject()->HasAnyFlags(RF_NeedLoad), "");

			/*
			for (FProperty* P = bCanUsePostConstructLink ? Class->PostConstructLink : Class->PropertyLink; P; P = 	bCanUsePostConstructLink ? P->PostConstructLinkNext : P->PropertyLinkNext)
			{
				if (bNeedInitialize)
				{
					bNeedInitialize = InitNonNativeProperty(P, Obj);
				}


				bool bIsTransient = P->HasAnyPropertyFlags(CPF_Transient | CPF_DuplicateTransient | CPF_NonPIEDuplicateTransient);
				if (!bIsTransient || !P->ContainsInstancedObjectProperty())
				{
					if (bCopyTransientsFromClassDefaults && bIsTransient)
					{
						// This is a duplicate. The value for all transient or non-duplicatable properties should be copied
						// from the source class's defaults.
						P->CopyCompleteValue_InContainer(Obj, ClassDefaults);
					}
					else if (P->IsInContainer(DefaultsClass))
					{
						P->CopyCompleteValue_InContainer(Obj, DefaultData);
					}
				}
			}
			*/


			// This step is only necessary if we're not iterating the full property chain.
			if (bCanUsePostConstructLink)
			{
				// Initialize remaining property values from defaults using an explicit custom post-construction property list returned 	by the class object.
				Class->InitPropertiesFromCustomList((uint8_t*)Obj, (uint8_t*)DefaultData);
			}
		}
	}

	void FObjectInitializer::PostConstructInit()
	{
		// we clear the m_Object pointer at the end of this function, so if it is null
		// then it most likely means that this is being ran for a second time
		if (m_Object == nullptr)
		{
	#if USE_DEFERRED_DEPENDENCY_CHECK_VERIFICATION_TESTS
			checkf(Obj != nullptr, TEXT("Looks like you're attempting to run FObjectInitializer::PostConstructInit() twice, and that 	should never happen."));
	#endif // USE_DEFERRED_DEPENDENCY_CHECK_VERIFICATION_TESTS
			return;
		}

		//SCOPE_CYCLE_COUNTER(STAT_PostConstructInitializeProperties);
		const bool bIsCDO = m_Object->HasAnyFlags(RF_ClassDefaultObject);
		UClass* Class = m_Object->GetClass();
		UClass* SuperClass = Class->GetSuperClass();

		if (m_bShouldInitializePropsFromArchetype)
		{
			UClass* BaseClass = (bIsCDO /*&& !GIsDuplicatingClassForReinstancing*/) ? SuperClass : Class;
			if (BaseClass == NULL)
			{
				KR_CORE_ASSERT(Class == UObject::StaticClass(), "");
				BaseClass = Class;
			}

			UObject* Defaults = m_ObjectArchetype ? m_ObjectArchetype : BaseClass->GetDefaultObject(false); // we don't create the CDO 	here if it doesn't already exist
			InitProperties(m_Object, BaseClass, Defaults, m_bCopyTransientsFromClassDefaults);
		}


	#if USE_CIRCULAR_DEPENDENCY_LOAD_DEFERRING
		//const bool bAllowInstancing = IsInstancingAllowed() && !bIsDeferredInitializer;
	#else
		//const bool bAllowInstancing = IsInstancingAllowed();
	#endif // USE_CIRCULAR_DEPENDENCY_LOAD_DEFERRING
		//bool bNeedSubobjectInstancing = InitSubobjectProperties(bAllowInstancing);


		// Restore class information if replacing native class.
		/*if (ObjectRestoreAfterInitProps != NULL)
		{
			ObjectRestoreAfterInitProps->Restore();
			delete ObjectRestoreAfterInitProps;
			ObjectRestoreAfterInitProps = NULL;
		}*/


		bool bNeedInstancing = false;
		// if HasAnyFlags(RF_NeedLoad), we do these steps later
	#if !USE_CIRCULAR_DEPENDENCY_LOAD_DEFERRING
		if (!m_Object->HasAnyFlags(RF_NeedLoad))
	#else
		// we defer this initialization in special set of cases (when Obj is a CDO
		// and its parent hasn't been serialized yet)... in those cases, Obj (the
		// CDO) wouldn't have had RF_NeedLoad set (not yet, because it is created
		// from Class->GetDefualtObject() without that flag); since we've deferred
		// all this, it is likely that this flag is now present... these steps
		// (specifically sub-object instancing) is important for us to run on the
		// CDO, so we allow all this when the bIsDeferredInitializer is true as well
		if (!Obj->HasAnyFlags(RF_NeedLoad) || bIsDeferredInitializer)
	#endif // !USE_CIRCULAR_DEPENDENCY_LOAD_DEFERRING
		{
			if (bIsCDO || Class->HasAnyClassFlags(CLASS_PerObjectConfig))
			{
				//m_Object->LoadConfig(NULL, NULL, bIsCDO ? UE::LCPF_ReadParentSections : UE::LCPF_None);
			}/*
			if (bAllowInstancing)
			{
				// Instance subobject templates for non-cdo blueprint classes or when using non-CDO template.
				const bool bInitPropsWithArchetype = Class->GetDefaultObject(false) == NULL || Class->GetDefaultObject(false) != 	ObjectArchetype || Class->HasAnyClassFlags(CLASS_CompiledFromBlueprint);
				if ((!bIsCDO || bShouldInitializePropsFromArchetype) && Class->HasAnyClassFlags(CLASS_HasInstancedReference) && 	bInitPropsWithArchetype)
				{
					// Only blueprint generated CDOs can have their subobjects instanced.
					check(!bIsCDO || !Class->HasAnyClassFlags(CLASS_Intrinsic|CLASS_Native));


					bNeedInstancing = true;
				}
			}*/
		}


		// Allow custom property initialization to happen before PostInitProperties is called
		/*if (PropertyInitCallback)
		{
			PropertyInitCallback();
		}*/
		// After the call to `PropertyInitCallback` to allow the callback to modify the instancing graph
		/*if (bNeedInstancing || bNeedSubobjectInstancing)
		{
			InstanceSubobjects(Class, bNeedInstancing, bNeedSubobjectInstancing);
		}*/


		// Make sure subobjects knows that they had their properties overwritten
		/*for (int32 Index = 0; Index < ComponentInits.SubobjectInits.Num(); Index++)
		{
			SCOPE_CYCLE_COUNTER(STAT_PostReinitProperties);
			UObject* Subobject = ComponentInits.SubobjectInits[Index].Subobject;
			Subobject->PostReinitProperties();
		}


		{
			SCOPE_CYCLE_COUNTER(STAT_PostInitProperties);
			Obj->PostInitProperties();
		}*/


		//Class->PostInitInstance(Obj, InstanceGraph);


	/*#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		if (!FUObjectThreadContext::Get().PostInitPropertiesCheck.Num() || 	(FUObjectThreadContext::Get().PostInitPropertiesCheck.Pop(false) != Obj))
		{
			UE_LOG(LogUObjectGlobals, Fatal, TEXT("%s failed to route PostInitProperties. Call Super::PostInitProperties() in 	%s::PostInitProperties()."), *Obj->GetClass()->GetName(), *Obj->GetClass()->GetName());
		}
	#endif */// !(UE_BUILD_SHIPPING || UE_BUILD_TEST)


	#if !USE_CIRCULAR_DEPENDENCY_LOAD_DEFERRING
		if (!m_Object->HasAnyFlags(RF_NeedLoad)
	#else
		// we defer this initialization in special set of cases (when Obj is a CDO
		// and its parent hasn't been serialized yet)... in those cases, Obj (the
		// CDO) wouldn't have had RF_NeedLoad set (not yet, because it is created
		// from Class->GetDefualtObject() without that flag); since we've deferred
		// all this, it is likely that this flag is now present... we want to run
		// all this as if the object was just created, so we check
		// bIsDeferredInitializer as well
		if ( (!Obj->HasAnyFlags(RF_NeedLoad) || bIsDeferredInitializer)
	#endif // !USE_CIRCULAR_DEPENDENCY_LOAD_DEFERRING
			// if component instancing is not enabled, then we leave the components in an invalid state, which will presumably be fixed 	by the caller
			/*&& ((InstanceGraph == NULL) || InstanceGraph->IsSubobjectInstancingEnabled())*/)
		{
			//m_Object->CheckDefaultSubobjects();
		}

		m_Object->ClearFlags(RF_NeedInitialization);

		// clear the object pointer so we can guard against running this function again
		m_Object = nullptr;
	}

	UObject* StaticFindObjectFastInternal(const UClass* ObjectClass, const UObject* ObjectPackage, const std::string& ObjectName, bool bExactClass, EObjectFlags ExcludeFlags, EInternalObjectFlags ExclusiveInternalFlags)
	{
		UObject* result = nullptr;

		if (ObjectPackage != nullptr)
		{
			// We start with the primitive type of vector iterations
			// how about const referencing element
			for (auto& element : GUObjectStore)
			{
				if (element->m_Object->GetName() == ObjectName

					/* Don't return objects that have any of the exclusive flags set */
					&& !element->m_Object->HasAnyFlags(ExcludeFlags)

					/* check that the object has the correct Outer */
					&& element->m_Object->GetOuter() == ObjectPackage

					/** If a class was specified, check that the object is of the correct class (hierarchy) */
					&& (ObjectClass == nullptr || (bExactClass ? element->m_Object->GetClass() == ObjectClass : element->m_Object->IsA(ObjectClass)))

					/** Include (or not) pending kill objects */
					// leaving for now. may become relevant later
					)
				{
					if (result)
					{
						KR_CORE_WARN("Ambigous search, could be {0} or {1}", result->GetName(), element->m_Object->GetName());
					}
					else
					{
						result = (UObject*)element->m_Object;
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
		if(Result == nullptr)
		{
			KR_CORE_INFO("Could not allocate space for UObject");
			return nullptr;
		}

		KR_CORE_ASSERT(InClass->m_ClassConstructor, "No default constructor found");

		(*InClass->m_ClassConstructor)(FObjectInitializer(Result, /*InTemplate*/nullptr, Params.m_bCopyTransientsFromClassDefaults, true, /*Params.InstanceGraph*/nullptr));

		{
			// Since the placement new (called in the constructor above, m_ClassConstructor) resets the m_NamePrivate and m_ObjectFlags
			// we set them again. Seems jugaadu and no offsets are created.
			
			Result->SetObjectName(InName);
			Result->SetFlags(EObjectFlags(InFlags | RF_NeedInitialization));

			Result->SetInternalFlags(Params.m_InternalSetFlags);
		}

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

		UObjectBase* ObjectBase = nullptr;
		UObject* Object = nullptr;
		std::string objectName = inName;

		if (objectName == "")
		{
			objectName = "NoName";
			KR_CORE_WARN("Attempting to create UObject with empty name string. Defaulting to NoName");
		}

		// see if object already exists in GUObjectStore. Maybe not functional
		Object = StaticFindObjectFastInternal(nullptr, inOuter, objectName, true);

		size_t totalSize = inClass->GetPropertiesSize();

		KR_CORE_ASSERT(totalSize, "Empty data structure");

		if (Object == nullptr)
		{
			size_t Alignment = FMath::Max<size_t>(4, inClass->GetMinAlignment());

			// Following corresponds to the instantiation of UObjects
			// from Karma's memory system known by the name Smriti.

			// I am using first reinterpret cast to void and then to UObject pointer
			// because direct reinterpret cast to UObject gives a warning in AppleClang
			void* aPtr = reinterpret_cast<void*>(GUObjectAllocator.AllocateUObject(totalSize, Alignment, GIsInitialLoad));
			FMemory::Memzero(aPtr, totalSize);

			ObjectBase = (UObjectBase*)aPtr;
		}
		else
		{
			KR_CORE_INFO("UObject with name {0} already exists. Won't create new.", objectName);
			return nullptr;
		}

		EObjectFlags relevantFlags = EObjectFlags (inFlags | RF_NeedInitialization);

		// Following UE, we first call the UObjectBase constructor
		new (ObjectBase) UObjectBase(const_cast<UClass*>(inClass), relevantFlags, internalSetFlags, inOuter, inName);

		Object = (UObject*)ObjectBase;

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

	void FUObjectArray::AddUObject(UObject* Object)
	{
		FUObjectItem* ObjectItem = new FUObjectItem();
		
		/**
		 * Taken from Game Coding Complete 4th edition, page 169. 
		 * There is variety of ways for indexing the UObjects, for simplicity we will start with number
		 * and based upon the complexity (if any?) we may transition to more appropriate indexing scheme
		 * as per the need.
		 */
		Object->SetInternalIndex(GUObjectStore.Num());
		ObjectItem->m_Object = Object;

		Add(ObjectItem);
	}

	void GetObjectsOfClass(const UClass* ClassToLookFor, KarmaVector<UObject *>& Results, bool bIncludeDerivedClasses, EObjectFlags ExclusionFlags, EInternalObjectFlags ExclusionInternalFlags)
	{
		//SCOPE_CYCLE_COUNTER(STAT_Hash_GetObjectsOfClass);

		ForEachObjectOfClass(ClassToLookFor,
			[&Results](UObject* Object)
			{
				Results.Add(Object);
			}
		, bIncludeDerivedClasses, ExclusionFlags, ExclusionInternalFlags);

		KR_CORE_ASSERT(Results.Num() <= GUObjectStore.Num(), ""); // otherwise we have a cycle in the outer chain, which should not be possible
	}

	KarmaVector<UObject*>* KarmaClassObjectMap::FindClassObjects(const UClass* Key)
	{
		// see if iterators can be used
		for(auto iterator = m_KeyValuePair.begin(); iterator != m_KeyValuePair.end(); iterator++)
		{
			if(iterator->first->GetName() == Key->GetName())// again jugaad class name comparison, also see UStruct::IsChildOf
			{
				return iterator->second;
			}
		}
		return nullptr;
	}

	KarmaVector<UObject*>* KarmaClassObjectMap::FindOrAddClass(const UClass* Key)
	{
		// see if iterators can be used
		for(auto iterator = m_KeyValuePair.begin(); iterator != m_KeyValuePair.end(); iterator++)
		{
			if(iterator->first->GetName() == Key->GetName())// again jugaad class name comparison, also seeUStruct::IsChildOf
			{
				return iterator->second;
			}
		}

		// Specified UClass doesn't exist yet, so add one
		UClass* aKey = const_cast<UClass*>(Key);
		KarmaVector<UObject*>* objects = new KarmaVector<UObject*>();// +++++++++ memory management needed here +++++++

		std::pair<UClass*, KarmaVector<UObject*>*> aPair = std::make_pair(aKey, objects);

		// Add the (key, value) pair
		m_KeyValuePair.emplace(aPair);

		return objects;
	}

	void ForEachObjectOfClass(const UClass* ClassToLookFor, std::function<void(UObject*)> Operation, bool bIncludeDerivedClasses, EObjectFlags ExclusionFlags, EInternalObjectFlags ExclusionInternalFlags)
	{
		//TRACE_CPUPROFILER_EVENT_SCOPE(ForEachObjectOfClass)

		KarmaVector<UObject*>* objectVector = m_ClassToObjectVectorMap.FindClassObjects(ClassToLookFor);

		if(objectVector != nullptr)
		{
			for(UObject* Object : *objectVector)
			{
				if (!Object->HasAnyFlags(ExclusionFlags) && !Object->HasAnyInternalFlags(ExclusionInternalFlags))
				{
					Operation(Object);
				}
			}
		}
		else
		{
			KR_CORE_INFO("Could not find any objects corresponding to class {0}", ClassToLookFor->GetName());
		}
	}

	void CacheObject(UObject* Object)
	{
		const UClass* classToLookFor = const_cast<UClass*>(Object->GetClass());

		KarmaVector<UObject*>* objectVector = m_ClassToObjectVectorMap.FindOrAddClass(classToLookFor);
		objectVector->Add(Object);
	}
}
