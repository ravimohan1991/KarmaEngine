#include "Actor.h"
#include "GameFramework/Level.h"

namespace Karma
{
	AActor::AActor()
	{
	}

	ULevel* AActor::GetLevel() const
	{
		return GetTypedOuter<ULevel>();
	}
}