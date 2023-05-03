#include "World.h"
#include "Core/UObjectGlobals.h"
#include "GameFramework/Actor.h"
#include "Core/Class.h"
#include "GameFramework/Level.h"
#include "Ganit/Transform.h"
#include "Level.h"
#include "Karma/Core/Package.h"

namespace Karma
{
	FActorSpawnParameters::FActorSpawnParameters()
	{
		m_Name = "NoName";
		m_Owner = nullptr;
		m_Instigator = nullptr;
		m_Template = nullptr;
		m_OverrideLevel = nullptr;
	}

	UWorld::UWorld() : UObject()
	{
		m_TimeSeconds = 0.0f;
		m_CurrentLevel = nullptr;
		m_PersistentLevel = nullptr;
		m_bIsTearingDown = false;

		GenerateLevel();
	}

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

		ULevel* LevelToSpawnIn = spawnParameters.m_OverrideLevel;

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

		LevelToSpawnIn->m_Actors.Add(Actor);
		//LevelToSpawnIn->ActorsForGC.Add(Actor);

		Actor->PostSpawnInitialize(UserTransform, spawnParameters.m_Owner, spawnParameters.m_Instigator, spawnParameters.IsRemoteOwned(), spawnParameters.m_bNoFail, spawnParameters.m_bDeferConstruction);

		return Actor;
	}

	UWorld* UWorld::CreateWorld(const EWorldType::Type InWorldType, bool bInformEngineOfWorld, const std::string& WorldName, UPackage* InWorldPackage, bool bAddToRoot,/* ERHIFeatureLevel::Type InFeatureLevel = ERHIFeatureLevel::Num, const InitializationValues* InIVS = nullptr,*/ bool bInSkipInitWorld)
	{
		//TRACE_CPUPROFILER_EVENT_SCOPE(UWorld::CreateWorld);

		UPackage* WorldPackage = InWorldPackage;
		if (!WorldPackage)
		{
			WorldPackage = CreatePackage("XPackage");// nullptr);
		}

		if (InWorldType == EWorldType::PIE)
		{
			WorldPackage->SetPackageFlags(PKG_PlayInEditor);
		}

		// Mark the package as containing a world.  This has to happen here rather than at serialization time,
		// so that e.g. the referenced assets browser will work correctly.
		//if (WorldPackage != GetTransientPackage())
		//{
			WorldPackage->ThisContainsMap();
		//}

		// Create new UWorld, ULevel and UModel.
		std::string WorldNameString;
		if (WorldName == "")
		{
			WorldNameString = "NoName";
		}
		else
		{
			WorldNameString = WorldName;
		}

		UWorld* NewWorld = NewObject<UWorld>(WorldPackage, UWorld::StaticClass<UWorld>(), WorldNameString);

		NewWorld->SetFlags(RF_Transactional);
		NewWorld->m_WorldType = InWorldType;
		//NewWorld->FeatureLevel = InFeatureLevel;
		//NewWorld->InitializeNewWorld(InIVS ? *InIVS : UWorld::InitializationValues().CreatePhysicsScene(InWorldType != EWorldType::Inactive).ShouldSimulatePhysics(false).EnableTraceCollision(true).CreateNavigation(InWorldType == EWorldType::Editor).CreateAISystem(InWorldType == EWorldType::Editor), bInSkipInitWorld);

		// Clear the dirty flags set during SpawnActor and UpdateLevelComponents
		//WorldPackage->SetDirtyFlag(false);
		//for (UPackage* ExternalPackage : WorldPackage->GetExternalPackages())
		//{
		//	ExternalPackage->SetDirtyFlag(false);
		//}

		if (bAddToRoot)
		{
			// Add to root set so it doesn't get garbage collected.
			NewWorld->AddToRoot();
		}

		// Tell the engine we are adding a world (unless we are asked not to)
		/*
		if ((GEngine) && (bInformEngineOfWorld == true))
		{
			GEngine->WorldAdded(NewWorld);
		}*/

		return NewWorld;
	}

	bool UWorld::ShivaActor(AActor* ThisActor, bool bNetForce, bool bShouldModifyLevel)
	{
		// Not yet functional. Only placeholder till we get the spawn process satisfactory
		// Return success.
		return true;
	}

	bool UWorld::AreActorsInitialized() const
	{
		return m_bActorsInitialized && m_PersistentLevel && m_PersistentLevel->m_Actors.Num();
	}

	bool UWorld::HasBegunPlay() const
	{
		return m_bBegunPlay && m_PersistentLevel && m_PersistentLevel->m_Actors.Num();
	}

	void UWorld::GenerateLevel()
	{
		m_CurrentLevel = new ULevel();
		m_CurrentLevel->m_OwningWorld = this;

		// Dereferencing required
	}
}
