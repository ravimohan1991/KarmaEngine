#include "ChildActorComponent.h"

namespace Karma
{
	void UChildActorComponent::BeginDestroy()
	{
		USceneComponent::BeginDestroy();

		/*
		if (m_CachedInstanceData)
		{
			delete CachedInstanceData;
			CachedInstanceData = nullptr;
		}
		*/
	}
}