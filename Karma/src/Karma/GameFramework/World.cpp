#include "World.h"
#include "Core/UObjectGlobals.h"
#include "GameFramework/Actor.h"
#include "Core/Class.h"
#include "GameFramework/Level.h"
#include "Ganit/Transform.h"

namespace Karma
{
	AActor* UWorld::SpawnActor(UClass* Class, FTransform const* transform, const FActorSpawnParameters& spawnParameters)
	{
		if (Class == nullptr)
		{
			KR_CORE_ERROR("SpawnActor failed because no class was specified");
			return nullptr;
		}

		// Remember this is jugaad reflection. In true reflection the syntax is AActor::StaticClass()
		if (!Class->IsChildOf(AActor::StaticClass<AActor>()))
		{
			KR_CORE_ERROR("SpawnActor failed because {0} is not an actor class", Class->GetName());
			return nullptr;
		}
		else if(spawnParameters.m_Template != nullptr && spawnParameters.m_Template->GetClass() != Class)
		{
			KR_CORE_ERROR("SpawnActor failed because template class {0} does not match spawn class {1}", spawnParameters.m_Template->GetClass()->GetName(), Class->GetName());
			return nullptr;
		}
		else if (m_bIsTearingDown)
		{
			KR_CORE_ERROR("SpawnActor failed because we are in the process of tearing down the world");
			return nullptr;
		}

		ULevel* LevelToSpawnIn =spawnParameters.m_OverrideLevel;

		if (LevelToSpawnIn == nullptr)
		{
			LevelToSpawnIn = (spawnParameters.m_Owner != nullptr) ? spawnParameters.m_Owner->GetLevel() : m_CurrentLevel;
		}

		std::string newActorName = spawnParameters.m_Name;
		EObjectFlags actorFlags = spawnParameters.m_ObjectFlags;
		// Use class's default actor as a template if none provided.
		UObject* aTemplate = spawnParameters.m_Template ? spawnParameters.m_Template : nullptr;//Class->GetDefaultObject<AActor>();

		FTransform const UserTransform = (transform != nullptr) ? *transform : FTransform::Identity();
		 
		AActor* const Actor = NewObject<AActor>(LevelToSpawnIn, Class, newActorName, actorFlags, aTemplate, false);

		LevelToSpawnIn->Actors.push_back(Actor);
		//LevelToSpawnIn->ActorsForGC.Add(Actor);

		return Actor;
	}
}