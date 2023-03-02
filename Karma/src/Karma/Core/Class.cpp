#include "Class.h"

namespace Karma
{
	UClass::UClass(const std::string& name) : m_NamePrivate(name)
	{
	}

	UClass* UField::GetOwnerClass() const
	{
		return nullptr;// for now
	}

	const std::string& UField::GetAuthoredName() const
	{
		return "NoName";
	}

	const std::string& UClass::GetDesc()
	{
		return GetName();
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
			if (TempStruct == SomeBase)
			{
				bOldResult = true;
				break;
			}
		}

		return bOldResult;
	}
}