#pragma once

#include "krpch.h"
#include "Object.h"

#include "GameFramework/SceneComponent.h"

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
		 * All ActorComponents owned by this Actor. Stored as a std::vector as actors may have a large number of components
		 * @see GetComponents()
		 */
		KarmaVector<std::shared_ptr<UActorComponent>> m_OwnedComponents;

		/** 
		 * Pawn responsible for damage and other gameplay events caused by this actor. 
		 * 
		 * @see AActor::SetInstigator(APawn* InInstigator)
		 */
		APawn* m_Instigator;

	public:
		/** Return the ULevel that this Actor is part of. */
		ULevel* GetLevel() const;

		/** Called after the actor is spawned in the world.  Responsible for setting up actor for play. */
		void PostSpawnInitialize(FTransform const& SpawnTransform, AActor* InOwner, APawn* InInstigator, bool bRemoteOwned, bool bNoFail, bool bDeferConstruction);

		/** Getter for the cached world pointer, will return null if the actor is not actually spawned in a level */
		virtual UWorld* GetWorld() const override final;

		/**
		 * Set the owner of this Actor, used primarily for network replication.
		 * @param NewOwner	The Actor who takes over ownership of this Actor
		 */
		virtual void SetOwner(AActor* NewOwner);

		/**
		 * See if this actor is owned by TestOwner.
		 * @see AActor::SetOwner(AActor* NewOwner)
		 */
		inline bool IsOwnedBy(const AActor* TestOwner) const
		{
			for (const AActor* Arg = this; Arg; Arg = Arg->m_Owner)
			{
				if (Arg == TestOwner)
				{
					return true;
				}
			}
			return false;
		}

		/**
		 * Get all components derived from class 'ComponentType' and fill in the OutComponents array with the result.
		 * It's recommended to use TArrays with a TInlineAllocator to potentially avoid memory allocation costs.
		 *
		 * @param bIncludeFromChildActors	If true then recurse in to ChildActor components and find components of the appropriate type in those Actors as well
		 */
		void GetComponents(KarmaVector<USceneComponent*>& OutComponents) const // make use of smartpointer ?
		{
			// We should consider removing this function.  It's not really hurting anything by existing but the one above it was added so that
			// we weren't assuming T*, preventing TObjectPtrs from working for this function.  The only downside is all the people who force the
			// template argument with GetComponents's code suddenly not compiling with no clear error message.

			OutComponents.SmartReset();

			// Our own implementation, different from UE, maybe sync in future
			typename std::vector<std::shared_ptr<UActorComponent>>::const_iterator iterator = m_OwnedComponents.GetElements().begin();

			USceneComponent* tempSceneComponent;

			while (iterator != m_OwnedComponents.GetElements().end())
			{
				tempSceneComponent = static_cast<USceneComponent*>((*iterator).get());
				if (tempSceneComponent != nullptr)
				{
					OutComponents.Add(tempSceneComponent);
				}
			}

			/*
			ForEachComponent_Internal<T>(T::StaticClass(), bIncludeFromChildActors, [&](T* InComp)
				{
					OutComponents.Add(InComp);
				});
				*/
		}

		/** Sets the value of Instigator without causing other side effects to this instance. */
		void SetInstigator(APawn* InInstigator);

		/** Returns this actor's root component. */
		FORCEINLINE USceneComponent* GetRootComponent() const { return m_RootComponent; }

		/**
		 * Sets root component to be the specified component.  NewRootComponent's owner should be this actor.
		 * @return true if successful
		 */
		bool SetRootComponent(USceneComponent* NewRootComponent);

		/**
		 * Get the actor-to-world transform.
		 * @return The transform that transforms from actor space to world space.
		 */
		const FTransform& GetTransform() const
		{
			return ActorToWorld();
		}

		/** Get the local-to-world transform of the RootComponent. Identical to GetTransform(). */
		inline const FTransform& ActorToWorld() const
		{
			return (m_RootComponent ? m_RootComponent->GetComponentTransform() : FTransform::Identity());
		}

	public:
		/**
		 * The time this actor was created, relative to World->GetTimeSeconds().
		 * @see UWorld::GetTimeSeconds()
		 */
		float m_CreationTime;

		/**
		 * Owner of this Actor, used primarily for replication (bNetUseOwnerRelevancy & bOnlyRelevantToOwner) and visibility (PrimitiveComponent bOwnerNoSee and bOnlyOwnerSee)
		 * @see SetOwner(), GetOwner()
		 */
		AActor* m_Owner;// UE uses smart pointer

		/** 
		 * Array of all Actors whose Owner is this actor, these are not necessarily spawned by UChildActorComponent 
		 * Needs to be raw pointer only.
		 */
		KarmaVector<AActor*> m_Children;

	protected:
		/** 
		 * The component that defines the transform (location, rotation, scale) of this Actor 
		 * in the world, all other components must be attached to this one somehow.
		 * 
		 * Needs be raw pointer only
		 */
		USceneComponent* m_RootComponent;
	};
}
