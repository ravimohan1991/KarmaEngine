#include "World.h"

namespace Karma
{
	AActor* UWorld::SpawnActor(FTransform const* transform, const FActorSpawnParameters& spawnParameters)
	{
		// A check to see if we are not spawning the child of AActor

		ULevel* LevelToSpawnIn = spawnParameters.m_OverrideLevel;

		if (LevelToSpawnIn == nullptr)
		{
			LevelToSpawnIn;// = spawnParameters.m_Owner.
		}

		return nullptr;
	}
}