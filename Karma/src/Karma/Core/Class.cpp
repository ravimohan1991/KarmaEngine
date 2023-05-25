#include "Class.h"
#include "KarmaMemory.h"

namespace Karma
{
	void UStruct::SetSuperStruct(UStruct* NewSuperStruct)
	{
		m_SuperStruct = NewSuperStruct;
/*
#if USTRUCT_FAST_ISCHILDOF_IMPL == USTRUCT_ISCHILDOF_STRUCTARRAY
		this->ReinitializeBaseChainArray();
#endif
*/
	}

	UClass::UClass()
	{
		m_PropertiesSize = 0;
		m_MinAlignment = 0;

		SetSuperStruct(nullptr);
		SetObjectName("NoName");
	}

	UClass::UClass(const std::string& name)
	{
		m_PropertiesSize = 0;
		m_MinAlignment = 0;

		SetSuperStruct(nullptr);
		SetObjectName(name);
	}

	UClass::UClass(const std::string& name, uint32_t size, uint32_t alignment)
	{
		m_PropertiesSize = size;
		m_MinAlignment = alignment;

		SetSuperStruct(nullptr);
		SetObjectName(name);
	}

	UClass* UField::GetOwnerClass() const
	{
		return nullptr;// for now
	}

	const std::string& UField::GetAuthoredName() const
	{
		static std::string someString = "NoName";
		return someString;
	}

	const std::string& UClass::GetDesc()
	{
		return GetName();
	}

	void UClass::SetSuperStruct(UStruct* NewSuperStruct)
	{
		/*UnhashObject(this);
		ClearFunctionMapsCaches();*/

		Super::SetSuperStruct(NewSuperStruct);

		/*if (!GetSparseClassDataStruct())
		{
			if (UScriptStruct* SparseClassDataStructArchetype = GetSparseClassDataArchetypeStruct())
			{
				SetSparseClassDataStruct(SparseClassDataStructArchetype);
			}
		}*/

		//HashObject(this);
	}

	/**
	 * @return	true if this object is of the specified type.
	 */
	bool UStruct::IsChildOf(const UStruct* SomeBase) const
	{
		// If you're looking at this check it is due to calling IsChildOf with a this nullptr. *MAKE* sure you do not call this function
		// with a this nullptr. It is undefined behavior, and some compilers, clang13 have started to optimize out this == nullptr checks.
		KR_CORE_ASSERT(this, "We don't want to call this on nullptr");

		if (SomeBase == nullptr)
		{
			return false;
		}

		bool bOldResult = false;
		for (const UStruct* TempStruct = this; TempStruct; TempStruct = TempStruct->GetSuperStruct())
		{
			if (TempStruct->GetName() == SomeBase->GetName()) // Jugaad for now by == operator overloading. Need to write registration system like in UE
			{
				bOldResult = true;
				break;
			}
		}

		return bOldResult;
	}

	bool UStruct::operator==(const UStruct& Comparable) const
	{
		if (GetName() == Comparable.GetName())
		{
			return true;
		}

		return false;
	}

	/**
	* Get the default object from the class, creating it if missing, if requested or under a few other circumstances
	* @return		the CDO for this class
	**/
	UObject* UClass::CreateDefaultObject()
	{
		/*
		if (m_ClassDefaultObject == nullptr)
		{
			KR_CORE_ASSERT(!m_bLayoutChanging, "Class named {0} creating its CDO while changing its layout", GetName());

			UClass* ParentClass = GetSuperClass();
			UObject* ParentDefaultObject = nullptr;
			if (ParentClass != nullptr)
			{
				UObjectForceRegistration(ParentClass);
				ParentDefaultObject = ParentClass->GetDefaultObject(); // Force the default object to be constructed if it isn't already
				check(GConfig);
				if (GEventDrivenLoaderEnabled && EVENT_DRIVEN_ASYNC_LOAD_ACTIVE_AT_RUNTIME)
				{
					check(ParentDefaultObject && !ParentDefaultObject->HasAnyFlags(RF_NeedLoad));
				}
			}

			if ((ParentDefaultObject != NULL) || (this == UObject::StaticClass<UObject>()))
			{
				// If this is a class that can be regenerated, it is potentially not completely loaded.  Preload and Link here to ensure we properly zero memory and read in properties for the CDO
				if (HasAnyClassFlags(CLASS_CompiledFromBlueprint) && (PropertyLink == NULL) && !GIsDuplicatingClassForReinstancing)
				{
					auto ClassLinker = GetLinker();
					if (ClassLinker)
					{
						if (!GEventDrivenLoaderEnabled)
						{
							UField* FieldIt = Children;
							while (FieldIt && (FieldIt->GetOuter() == this))
							{
								// If we've had cyclic dependencies between classes here, we might need to preload to ensure that we load the rest of the property chain
								if (FieldIt->HasAnyFlags(RF_NeedLoad))
								{
									ClassLinker->Preload(FieldIt);
								}
								FieldIt = FieldIt->Next;
							}
						}

						StaticLink(true);
					}
				}

				// in the case of cyclic dependencies, the above Preload() calls could end up 
				// invoking this method themselves... that means that once we're done with  
				// all the Preload() calls we have to make sure ClassDefaultObject is still 
				// NULL (so we don't invalidate one that has already been setup)
				if (m_ClassDefaultObject == NULL)
				{
					// RF_ArchetypeObject flag is often redundant to RF_ClassDefaultObject, but we need to tag
					// the CDO as RF_ArchetypeObject in order to propagate that flag to any default sub objects.
					ClassDefaultObject = StaticAllocateObject(this, GetOuter(), NAME_None, EObjectFlags(RF_Public | RF_ClassDefaultObject | RF_ArchetypeObject));
					check(ClassDefaultObject);
					// Register the offsets of any sparse delegates this class introduces with the sparse delegate storage
					for (TFieldIterator<FMulticastSparseDelegateProperty> SparseDelegateIt(this, EFieldIteratorFlags::ExcludeSuper, EFieldIteratorFlags::ExcludeDeprecated); SparseDelegateIt; ++SparseDelegateIt)
					{
						const FSparseDelegate& SparseDelegate = SparseDelegateIt->GetPropertyValue_InContainer(ClassDefaultObject);
						USparseDelegateFunction* SparseDelegateFunction = CastChecked<USparseDelegateFunction>(SparseDelegateIt->SignatureFunction);
						FSparseDelegateStorage::RegisterDelegateOffset(ClassDefaultObject, SparseDelegateFunction->DelegateName, (size_t)&SparseDelegate - (size_t)ClassDefaultObject);
					}
					EObjectInitializerOptions InitOptions = EObjectInitializerOptions::None;
					if (!HasAnyClassFlags(CLASS_Native | CLASS_Intrinsic))
					{
						// Blueprint CDOs have their properties always initialized.
						InitOptions |= EObjectInitializerOptions::InitializeProperties;
					}
					(*ClassConstructor)(FObjectInitializer(ClassDefaultObject, ParentDefaultObject, InitOptions));
					if (GetOutermost()->HasAnyPackageFlags(PKG_CompiledIn) && !GetOutermost()->HasAnyPackageFlags(PKG_RuntimeGenerated))
					{
						TCHAR PackageName[FName::StringBufferSize];
						TCHAR CDOName[FName::StringBufferSize];
						GetOutermost()->GetFName().ToString(PackageName);
						GetDefaultObjectName().ToString(CDOName);
						NotifyRegistrationEvent(PackageName, CDOName, ENotifyRegistrationType::NRT_ClassCDO, ENotifyRegistrationPhase::NRP_Finished, nullptr, false, ClassDefaultObject);
					}
					ClassDefaultObject->PostCDOContruct();
				}
			}
		}*/
		//m_ClassDefaultObject = new 
		return nullptr;//m_ClassDefaultObject;
	}
}
