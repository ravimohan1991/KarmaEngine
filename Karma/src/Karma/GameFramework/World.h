#pragma once

#include "krpch.h"

#include "Object.h"
#include "SubClassOf.h"

namespace Karma
{
	class AActor;
	class FTransform;
	struct FActorSpawnParameters;
	class APawn;
	class ULevel;
	class UClass;

	/* Struct of optional parameters passed to SpawnActor function(s). */
	struct KARMA_API FActorSpawnParameters
	{
		FActorSpawnParameters();

		/* A name to assign as the Name of the Actor being spawned. If no value is specified, the name of the spawned Actor will be automatically generated using the form [Class]_[Number]. */
		std::string m_Name;

		/* An Actor to use as a template when spawning the new Actor. The spawned Actor will be initialized using the property values of the template Actor. If left NULL the class default object (CDO) will be used to initialize the spawned Actor. */
		AActor* m_Template;

		/* The Actor that spawned this Actor. (Can be left as NULL). */
		AActor* m_Owner;

		/* The APawn that is responsible for damage done by the spawned Actor. (Can be left as NULL). */
		APawn* m_Instigator;

		/* The ULevel to spawn the Actor in, i.e. the Outer of the Actor. If left as NULL the Outer of the Owner is used. If the Owner is NULL the persistent level is used. */
		ULevel* m_OverrideLevel;

#if WITH_EDITOR
		/* The UPackage to set the Actor in. If left as NULL the Package will not be set and the actor will be saved in the same package as the persistent level. */
		class	UPackage* OverridePackage;

		/** The Guid to set to this actor. Should only be set when reinstancing blueprint actors. */
		FGuid	OverrideActorGuid;
#endif

		/* The parent component to set the Actor in. */
		class   UChildActorComponent* m_OverrideParentComponent;

		/** Method for resolving collisions at the spawn point. Undefined means no override, use the actor's setting. */
		// Need physics
		//ESpawnActorCollisionHandlingMethod SpawnCollisionHandlingOverride;

	private:

		//friend class UPackageMapClient;

		/* Is the actor remotely owned. This should only be set true by the package map when it is creating an actor on a client that was replicated from the server. */
		uint8_t m_bRemoteOwned : 1;

	public:

		/* Modes that SpawnActor can use the supplied name when it is not None. */
		enum class ESpawnActorNameMode : uint8_t
		{
			/* Fatal if unavailable, application will assert */
			Required_Fatal,

			/* Report an error return null if unavailable */
			Required_ErrorAndReturnNull,

			/* Return null if unavailable */
			Required_ReturnNull,

			/* If the supplied Name is already in use the generate an unused one using the supplied version as a base */
			Requested
		};

		/* In which way should SpawnActor should treat the supplied Name if not none. */
		ESpawnActorNameMode m_NameMode;

		/* Flags used to describe the spawned actor/object instance. */
		EObjectFlags m_ObjectFlags;

		/* Custom function allowing the caller to specific a function to execute post actor construction but before other systems see this actor spawn. */
		// maybe later
		//TFunction<void(AActor*)> CustomPreSpawnInitalization;

		bool IsRemoteOwned() const { return m_bRemoteOwned; }

		/* Determines whether spawning will not fail if certain conditions are not met. If true, spawning will not fail because the class being spawned is `bStatic=true` or because the class of the template Actor is not the same as the class of the Actor being spawned. */
		uint8_t m_bNoFail : 1;

		/* Determines whether the construction script will be run. If true, the construction script will not be run on the spawned Actor. Only applicable if the Actor is being spawned from a Blueprint. */
		uint8_t m_bDeferConstruction : 1;
	};

	/**
	 * The World is the top level object representing a map or a sandbox in which Actors and Components will exist and be rendered.
	 *
	 * A World can be a single Persistent Level with an optional list of streaming levels that are loaded and unloaded via volumes and blueprint functions
	 * or it can be a collection of levels organized with a World Composition.
	 *
	 * In a standalone game, generally only a single World exists except during seamless area transitions when both a destination and current world exists.
	 * In the editor many Worlds exist: The level being edited, each PIE instance, each editor tool which has an interactive rendered viewport, and many more.
	 *
	 */
	class KARMA_API UWorld : public UObject
	{
		DECLARE_KARMA_CLASS(UWorld, UObject)

	public:
		UWorld();

		/**
		 * Spawn Actors with given transform and SpawnParameters
		 *
		 * @param	Class					Karma's, UE based, meta info Class to Spawn
		 * @param	Transform				World Transform to spawn with
		 * @param	SpawnParameters			Spawn Parameters specific to the Actor
		 *
		 * @return	Actor that just spawned
		 */
		AActor* SpawnActor(UClass* Class, FTransform const* Transform, const FActorSpawnParameters& SpawnParameters = FActorSpawnParameters());

		/** 
		 * Templated version of SpawnActor that allows you to specify the class type 
		 * via parameter while the return type is a parent class of that type 
		 */
		template< class T >
		T* SpawnActor(UClass* Class, const FActorSpawnParameters& SpawnParameters = FActorSpawnParameters())
		{
			return CastChecked<T>(SpawnActor(Class, nullptr, SpawnParameters), ECastCheckedType::NullAllowed);
		}

	private:
//#if WITH_EDITORONLY_DATA
		/** Pointer to the current level being edited. Level has to be in the Levels array and == PersistentLevel in the game. */
		ULevel*								m_CurrentLevel;
//#endif

		/** Persistent level containing the world info, default brush and actors spawned during gameplay among other things			*/
		ULevel*								m_PersistentLevel;

	public:
		/** Is the world being torn down */
		uint8_t m_bIsTearingDown : 1;

		/**  Time in seconds since level began play, but IS paused when the game is paused, and IS dilated/clamped. */
		double m_TimeSeconds;

		/** Whether actors have been initialized for play */
		uint8_t m_bActorsInitialized : 1;

		/** Whether BeginPlay has been called on actors */
		uint8_t m_bBegunPlay : 1;

		/** The type of world this is. Describes the context in which it is being used (Editor, Game, Preview etc.) */
		EWorldType::Type m_WorldType;

		/** The current renderer feature level of this world */
		//TEnumAsByte<ERHIFeatureLevel::Type> FeatureLevel;

		/** Struct containing a collection of optional parameters for initialization of a World. */
		struct InitializationValues
		{
			InitializationValues()
				: bInitializeScenes(true)
				, bAllowAudioPlayback(true)
				, bRequiresHitProxies(true)
				, bCreatePhysicsScene(true)
				, bCreateNavigation(true)
				, bCreateAISystem(true)
				, bShouldSimulatePhysics(true)
				, bEnableTraceCollision(false)
				, bForceUseMovementComponentInNonGameWorld(false)
				, bTransactional(true)
				, bCreateFXSystem(true)
				, bCreateWorldPartition(false)
			{
			}

			/** Should the scenes (physics, rendering) be created. */
			uint32_t bInitializeScenes : 1;

			/** Are sounds allowed to be generated from this world. */
			uint32_t bAllowAudioPlayback : 1;

			/** Should the render scene create hit proxies. */
			uint32_t bRequiresHitProxies : 1;

			/** Should the physics scene be created. bInitializeScenes must be true for this to be considered. */
			uint32_t bCreatePhysicsScene : 1;

			/** Should the navigation system be created for this world. */
			uint32_t bCreateNavigation : 1;

			/** Should the AI system be created for this world. */
			uint32_t bCreateAISystem : 1;

			/** Should physics be simulated in this world. */
			uint32_t bShouldSimulatePhysics : 1;

			/** Are collision trace calls valid within this world. */
			uint32_t bEnableTraceCollision : 1;

			/** Special flag to enable movement component in non game worlds (see UMovementComponent::OnRegister) */
			uint32_t bForceUseMovementComponentInNonGameWorld : 1;

			/** Should actions performed to objects in this world be saved to the transaction buffer. */
			uint32_t bTransactional : 1;

			/** Should the FX system be created for this world. */
			uint32_t bCreateFXSystem : 1;

			/** Should the world be partitioned */
			uint32_t bCreateWorldPartition : 1;

			/** The default game mode for this world (if any) */
			TSubclassOf<class AGameModeBase> DefaultGameMode;

			InitializationValues& InitializeScenes(const bool bInitialize) { bInitializeScenes = bInitialize; return *this; }
			InitializationValues& AllowAudioPlayback(const bool bAllow) { bAllowAudioPlayback = bAllow; return *this; }
			InitializationValues& RequiresHitProxies(const bool bRequires) { bRequiresHitProxies = bRequires; return *this; }
			InitializationValues& CreatePhysicsScene(const bool bCreate) { bCreatePhysicsScene = bCreate; return *this; }
			InitializationValues& CreateNavigation(const bool bCreate) { bCreateNavigation = bCreate; return *this; }
			InitializationValues& CreateAISystem(const bool bCreate) { bCreateAISystem = bCreate; return *this; }
			InitializationValues& ShouldSimulatePhysics(const bool bInShouldSimulatePhysics) { bShouldSimulatePhysics = bInShouldSimulatePhysics; return *this; }
			InitializationValues& EnableTraceCollision(const bool bInEnableTraceCollision) { bEnableTraceCollision = bInEnableTraceCollision; return *this; }
			InitializationValues& ForceUseMovementComponentInNonGameWorld(const bool bInForceUseMovementComponentInNonGameWorld) { bForceUseMovementComponentInNonGameWorld = bInForceUseMovementComponentInNonGameWorld; return *this; }
			InitializationValues& SetTransactional(const bool bInTransactional) { bTransactional = bInTransactional; return *this; }
			InitializationValues& CreateFXSystem(const bool bCreate) { bCreateFXSystem = bCreate; return *this; }
			InitializationValues& CreateWorldPartition(const bool bCreate) { bCreateWorldPartition = bCreate; return *this; }
			InitializationValues& SetDefaultGameMode(TSubclassOf<class AGameModeBase> GameMode) { DefaultGameMode = GameMode; return *this; }
		};

	public:
		//////////////////////////////////////////////////////////////////////////
		// UWorld inlines:

		// Is not live atm
		FORCEINLINE double GetTimeSeconds() const
		{
			return m_TimeSeconds;
		}

		FORCEINLINE ULevel* GetCurrentLevel() const
		{
			return m_CurrentLevel;
		}

	public:
		/** Returns true if the actors have been initialized and are ready to start play */
		bool AreActorsInitialized() const;

		/**
		 * Initializes a newly created world.
		 * This and nearby functions (create destroy world for instance) should be useful
		 */
		//void InitializeNewWorld(const InitializationValues IVS = InitializationValues(), bool bInSkipInitWorld = false);

		/** Returns true if gameplay has already started, false otherwise. */
		bool HasBegunPlay() const;

		/**
		 * Static function that creates a new UWorld and returns a pointer to it
		 */
		static UWorld* CreateWorld(const EWorldType::Type InWorldType, bool bInformEngineOfWorld, const std::string& WorldName = "NoName", UPackage* InWorldPackage = NULL, bool bAddToRoot = true,/* ERHIFeatureLevel::Type InFeatureLevel = ERHIFeatureLevel::Num, const InitializationValues* InIVS = nullptr,*/ bool bInSkipInitWorld = false);

		/**
		 * Initializes a newly created world.
		 */
		void InitializeNewWorld(const InitializationValues IVS = InitializationValues(), bool bInSkipInitWorld = false);

		/**
		 * Removes the actor from its level's actor list and generally cleans up the engine's internal state.
		 * What this function does not do, but is handled via garbage collection instead, is remove references
		 * to this actor from all other actors, and kill the actor's resources.  This function is set up so that
		 * no problems occur even if the actor is being destroyed inside its recursion stack.
		 * 
		 * In UE this routine is called DestoryActor, in Karma, obviously, we be using ShivaActor which should mean
		 * the same.
		 *
		 * @param	ThisActor				Actor to remove.
		 * @param	bNetForce				[optional] Ignored unless called during play.  Default is false.
		 * @param	bShouldModifyLevel		[optional] If true, Modify() the level before removing the actor.  Default is true.
		 * @return							true if destroyed or already marked for destruction, false if actor couldn't be destroyed.
		 * 
		 * @see Actor::DispatchBeginPlay(bool bFromLevelStreaming)
		 */
		bool ShivaActor(AActor* Actor, bool bNetForce = false, bool bShouldModifyLevel = true);
	};
}
