#include "UObjectBase.h"
#include "Core/Object.h"
#include "Core/Class.h"

namespace Karma
{
	void UObjectBase::MarkAsGarbage()
	{
	}

	void UObjectBase::ClearGarbage()
	{
	}

	/**
	 * Traverses the outer chain searching for the next object of a certain type.  (T must be derived from UObject)
	 *
	 * @param	Target class to search for
	 * @return	a pointer to the first object in this object's Outer chain which is of the correct type.
	 */
	UObject* UObjectBase::GetTypedOuter(UClass* Target) const
	{
		UObject* Result = nullptr;
		for (UObject* NextOuter = m_OuterPrivate; Result == nullptr && NextOuter != nullptr; NextOuter = NextOuter->GetOuter())
		{
			if (NextOuter->IsA(Target))
			{
				Result = NextOuter;
			}
		}
		return Result;
	}

	bool UObjectBase::IsChildOfWorkaround(const UClass* ObjClass, const UClass* TestCls)
	{
		return ObjClass->IsChildOf(TestCls);
	}
}