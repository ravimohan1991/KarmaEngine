#include "Object.h"
#include "Class.h"

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
}