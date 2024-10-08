#include "World.h"
#include "Core/UObjectGlobals.h"
#include "GameFramework/Actor.h"
#include "Core/Class.h"
#include "GameFramework/Level.h"
#include "Ganit/Transform.h"
#include "Level.h"
#include "Karma/Core/Package.h"
#include "WorldSettings.h"
#include "Engine/Engine.h"

namespace Karma
{
	FActorSpawnParameters::FActorSpawnParameters() : m_Name("NoName"), m_ObjectFlags(RF_Transactional),
		m_Owner(nullptr), m_Instigator(nullptr), m_Template(nullptr), m_OverrideLevel(nullptr)
	{
	}

	UWorld::UWorld() : UObject()
	{
		m_bIsPaused = false;
		m_TimeSeconds = 0.0f;
		m_UnpausedTimeSeconds = 0.0f;
		m_RealTimeSeconds = 0.0f;
		m_AudioTimeSeconds = 0.0f;
		m_DeltaRealTimeSeconds = 0.0f;
		m_DeltaTimeSeconds = 0.0f;
		m_PauseDelay = 0.0f;
		m_CurrentLevel = nullptr;
		m_PersistentLevel = nullptr;
		m_bIsTearingDown = false;
	}

	AActor* UWorld::SpawnActor(UClass* Class, FTransform const* transform, const FActorSpawnParameters& spawnParameters)
	{
		if (Class == nullptr)
		{
			KR_CORE_ERROR("SpawnActor failed because no class was specified");
			return nullptr;
		}

		// The Class hierarcy is traversed and m_NamePrivate is string compared
		if (!Class->IsChildOf(AActor::StaticClass()))
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

		if(Actor == nullptr)
		{
			KR_CORE_INFO("Couldn't spawn the Actor {0}. Please browse the logs", newActorName);
			return nullptr;
		}

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

		UWorld* NewWorld = NewObject<UWorld>(WorldPackage, UWorld::StaticClass(), WorldNameString);

		NewWorld->SetFlags(RF_Transactional);
		NewWorld->m_WorldType = InWorldType;
		//NewWorld->FeatureLevel = InFeatureLevel;
		NewWorld->InitializeNewWorld(UWorld::InitializationValues().CreatePhysicsScene(InWorldType != EWorldType::Inactive).ShouldSimulatePhysics(false).EnableTraceCollision(true).CreateNavigation(InWorldType == EWorldType::Editor).CreateAISystem(InWorldType == EWorldType::Editor), bInSkipInitWorld);

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
		if ((GEngine) && bInformEngineOfWorld)
		{
			GEngine->WorldAdded(NewWorld);
		}

		return NewWorld;
	}

	void UWorld::InitializeNewWorld(const InitializationValues IVS, bool bInSkipInitWorld)
	{
		if (!IVS.bTransactional)
		{
			ClearFlags(RF_Transactional);
		}

		m_PersistentLevel = NewObject<ULevel>(this, ULevel::StaticClass(), "PersistentLevel");
		
		FURL someURL("someurl");
		m_PersistentLevel->Initialize(someURL);
		
		// BSP (Binary Space Partitioning stuff)
		// m_PersistentLevel->Model = NewObject<UModel>(PersistentLevel);
		// m_PersistentLevel->Model->Initialize(nullptr, 1);
		
		m_PersistentLevel->m_OwningWorld = this;

		// Create the WorldInfo actor.
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.m_Name = "testWorldSettings";

		// Mark objects are transactional for undo/ redo.
		if (IVS.bTransactional)
		{
			SpawnInfo.m_ObjectFlags = (EObjectFlags) (SpawnInfo.m_ObjectFlags | RF_Transactional);
			m_PersistentLevel->SetFlags(RF_Transactional);
			//m_PersistentLevel->Model->SetFlags(RF_Transactional);
		}
		else
		{
			SpawnInfo.m_ObjectFlags = (EObjectFlags)(SpawnInfo.m_ObjectFlags & ~RF_Transactional);
			m_PersistentLevel->ClearFlags(RF_Transactional);
			//PersistentLevel->Model->ClearFlags(RF_Transactional);
		}

//#if WITH_EDITORONLY_DATA
		// Need to associate current level so SpawnActor doesn't complain.
		m_CurrentLevel = m_PersistentLevel;
//#endif

		//SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		// Set constant name for WorldSettings to make a network replication work between new worlds on host and client
		//SpawnInfo.Name = GEngine->WorldSettingsClass->GetFName();

		// Experimental
		UClass* worldSettingsClass = AWorldSettings::StaticClass();

		AWorldSettings* WorldSettings = SpawnActor<AWorldSettings>(worldSettingsClass, SpawnInfo);

		// Allow the world creator to override the default game mode in case they do not plan to load a level.
		//if (IVS.DefaultGameMode)
		//{
		//	WorldSettings->DefaultGameMode = IVS.DefaultGameMode;
		//}

		m_PersistentLevel->SetWorldSettings(WorldSettings);

		//check(GetWorldSettings());

#if WITH_EDITOR
		WorldSettings->SetIsTemporarilyHiddenInEditor(true);

		// Check if newly created world should be partitioned
		if (IVS.bCreateWorldPartition)
		{
			// World partition always uses actor folder objects
			FLevelActorFoldersHelper::SetUseActorFolders(PersistentLevel, true);
			PersistentLevel->ConvertAllActorsToPackaging(true);

			check(!GetStreamingLevels().Num());

			UWorldPartition::CreateOrRepairWorldPartition(WorldSettings);
		}
#endif

		/*if (!bInSkipInitWorld)
		{
			// Initialize the world
			InitWorld(IVS);

			// Update components.
			const bool bRerunConstructionScripts = !FPlatformProperties::RequiresCookedData();
			UpdateWorldComponents(bRerunConstructionScripts, false);
		}*/

		// JUGAAD for experimental purposes only
		InitializeActorsForPlay(FURL());
		m_bBegunPlay = true;
	}

	void UWorld::ModifyLevel(ULevel* Level) const
	{
		if (Level && Level->HasAnyFlags(RF_Transactional))
		{
			Level->Modify(false);
			//Level->Model->Modify(false); some BSP stuff
		}
	}

	bool UWorld::IsGameWorld() const
	{
		return m_WorldType == EWorldType::Game || m_WorldType == EWorldType::PIE || m_WorldType == EWorldType::GamePreview || m_WorldType == EWorldType::GameRPC;
	}

	void UWorld::RemoveActor(AActor* Actor, bool bShouldModifyLevel) const
	{
		if (ULevel* CheckLevel = Actor->GetLevel())
		{
			const int32 ActorListIndex = CheckLevel->m_Actors.Find(Actor);

			// Search the entire list.
			if (ActorListIndex != INDEX_NONE)
			{
				if (bShouldModifyLevel /* && GUndo*/)
				{
					ModifyLevel(CheckLevel);
				}

				if (!IsGameWorld())
				{
					CheckLevel->m_Actors.GetElements()[ActorListIndex]->Modify();
				}

				CheckLevel->m_Actors.SetVectorElementByIndex(ActorListIndex, nullptr);

				//CheckLevel->ActorsForGC.RemoveSwap(Actor);
			}
		}

		// Remove actor from network list
		// RemoveNetworkActor(Actor);
	}

	void UWorld::InitializeActorsForPlay(const FURL& InURL, bool bResetTime)
	{
		// KR_CORE_ASSERT(bIsWorldInitialized, "");

		// m_URL = InURL;

		if (!AreActorsInitialized())
		{
			m_bActorsInitialized = true;

			// Spawn server actors
		}
	}

	void UWorld::Tick(float DeltaSeconds)
	{
		// Update time.
		m_RealTimeSeconds += DeltaSeconds;

		if(!m_bIsPaused)
		{
			m_TimeSeconds += DeltaSeconds;
		}

		// Save off actual delta
		float RealDeltaSeconds = DeltaSeconds;

		// We digress for UE for sake of building with simplicity
		for (const auto& element : m_CurrentLevel->m_Actors)
		{
			if (element->CanTick())
			{
				element->Tick(DeltaSeconds);
			}
		}
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

	void UWorld::OnActorSpawned()
	{
		// Broadcast the event
	}

	void UWorld::AddOnActorSpawnedHandler() const
	{
		
	}
}
