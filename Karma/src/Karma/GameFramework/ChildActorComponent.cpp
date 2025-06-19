#include "ChildActorComponent.h"

namespace Karma
{
	void UChildActorComponent::BeginDestroy()
	{
		Super::BeginDestroy();

		/*
		if (m_CachedInstanceData)
		{
			delete CachedInstanceData;
			CachedInstanceData = nullptr;
		}
		*/
	}
}
