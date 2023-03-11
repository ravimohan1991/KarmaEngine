#pragma once

#include "krpch.h"
#include "Object.h"

namespace Karma
{
	class UActorComponent;
	class ULevel;
	class APawn;
	class FTransform;
	class UWorld;

	/**
	 * Actor is the base class for an Object that can be placed or spawned in a level. A concept taken from Unreal Engine.
	 * Actors may contain a collection of ActorComponents, which can be used to control how actors move, how they are rendered, etc.
	 */
	class KARMA_API AActor : public UObject
	{
	public:
		/** Default constructor for AActor */
		AActor();

	private:
		/**
		 * All ActorComponents owned by this Actor. Stored as a Set as actors may have a large number of components
		 * @see GetComponents()
		 */
		std::vector<std::shared_ptr<UActorComponent>> OwnedComponents;

	public:
		/** Return the ULevel that this Actor is part of. */
		ULevel* GetLevel() const;

		/** Called after the actor is spawned in the world.  Responsible for setting up actor for play. */
		void PostSpawnInitialize(FTransform const& SpawnTransform, AActor* InOwner, APawn* InInstigator, bool bRemoteOwned, bool bNoFail, bool bDeferConstruction);

		/** Getter for the cached world pointer, will return null if the actor is not actually spawned in a level */
		virtual UWorld* GetWorld() const override final;
	};
}
