#include "Field.h"
#include "Class.h"

namespace Karma
{
	FFieldClass::FFieldClass(const char* InCPPName, uint64_t InId, uint64_t InCastFlags, FFieldClass* InSuperClass, FField* (*ConstructFnPtr)(const FFieldVariant&, const std::string&, EObjectFlags))
		: ClassFlags(CLASS_None)
		, SuperClass(InSuperClass)
		, DefaultObject(nullptr)
		, ConstructFn(ConstructFnPtr)
	{
		//check(InCPPName);
		// Skip the 'F' prefix for the name
		//check(InCPPName[0] == 'F');
		//Name = ++InCPPName;

		//GetAllFieldClasses().Add(this);
		//GetNameToFieldClassMap().Add(Name, this);
	}

	void FField::Rename(const std::string& NewName)
	{
		m_NamePrivate = NewName;
		// @todo: What about FFieldPath now? (point by Epic people)
	}

	FField* FFieldClass::ConstructDefaultObject()
	{
		// No clue why UClass::StaticClass()->GetOutermost() is used in Construct as 1st parameter
		FFieldVariant someVariant;

		FField* NewDefault = Construct(someVariant, ("Default__%s", GetName()), EObjectFlags(RF_Transient | RF_ClassDefaultObject));
		return NewDefault;
	}

	/*
	FFieldClass* FField::StaticClass()
	{
		static FFieldClass StaticFieldClass(TEXT("FField"), FField::StaticClassCastFlagsPrivate(), FField::StaticClassCastFlags(), nullptr, &FField::Construct);
		return &StaticFieldClass;
	}
	*/
}
