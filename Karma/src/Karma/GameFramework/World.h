#pragma once

#include "krpch.h"

#include "Object.h"

class AActor;
class FTransform;
class FActorSpawnParameters;
class APawn;
class ULevel;

namespace Karma
{
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
		APawn* Instigator;

		/* The ULevel to spawn the Actor in, i.e. the Outer of the Actor. If left as NULL the Outer of the Owner is used. If the Owner is NULL the persistent level is used. */
		ULevel* m_OverrideLevel;

#if WITH_EDITOR
		/* The UPackage to set the Actor in. If left as NULL the Package will not be set and the actor will be saved in the same package as the persistent level. */
		class	UPackage* OverridePackage;

		/** The Guid to set to this actor. Should only be set when reinstancing blueprint actors. */
		FGuid	OverrideActorGuid;
#endif

		/* The parent component to set the Actor in. */
		class   UChildActorComponent* OverrideParentComponent;

		/** Method for resolving collisions at the spawn point. Undefined means no override, use the actor's setting. */
		// Need physics
		//ESpawnActorCollisionHandlingMethod SpawnCollisionHandlingOverride;

	private:

		//friend class UPackageMapClient;

		/* Is the actor remotely owned. This should only be set true by the package map when it is creating an actor on a client that was replicated from the server. */
		//uint8_t	bRemoteOwned : 1;

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
		ESpawnActorNameMode NameMode;

		/* Flags used to describe the spawned actor/object instance. */
		EObjectFlags ObjectFlags;

		/* Custom function allowing the caller to specific a function to execute post actor construction but before other systems see this actor spawn. */
		// maybe later
		//TFunction<void(AActor*)> CustomPreSpawnInitalization;
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
	public:
		/**
		 * Spawn Actors with given transform and SpawnParameters
		 *
		 * @param	Class					Class to Spawn -> Shall be used when reflection system is ready
		 * @param	Transform				World Transform to spawn on
		 * @param	SpawnParameters			Spawn Parameters
		 *
		 * @return	Actor that just spawned
		 */
		AActor* SpawnActor(FTransform const* Transform, const FActorSpawnParameters& SpawnParameters = FActorSpawnParameters());
	};
}