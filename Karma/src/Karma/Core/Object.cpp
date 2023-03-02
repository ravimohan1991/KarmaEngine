#include "Object.h"
#include "Class.h"

namespace Karma
{
	void UObject::FinishDestroy()
	{
	}

	UClass* UObject::StaticClass()
	{
		UClass StaticClassClass("UClass");
		return &StaticClassClass;
	}
}