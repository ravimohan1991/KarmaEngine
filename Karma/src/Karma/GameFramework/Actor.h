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
	class UChildActorComponent;

	/**
	 * Actor is the base class for an Object that can be placed or spawned in a level. A concept taken from Unreal Engine.
	 * Actors may contain a collection of ActorComponents, which can be used to control how actors move, how they are rendered, etc.
	 */
	class KARMA_API AActor : public UObject
	{
		DECLARE_KARMA_CLASS(AActor, UObject)

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

		/** 
		 * Whether FinishSpawning has been called for this Actor.  If it has not, the Actor is in a malformed state 
		 */
		uint8_t m_bHasFinishedSpawning : 1;

		/**
		 *	Indicates that PreInitializeComponents/PostInitializeComponents have been called on this Actor
		 *	Prevents re-initializing of actors spawned during level startup
		 */
		uint8_t m_bActorInitialized : 1;

		/**
		 * @see AActor::PostActorConstruction()
		 */
		static uint32_t m_BeginPlayCallDepth;


		/** The UChildActorComponent that owns this Actor. */
		// TWeakObjectPtr in UE
		std::shared_ptr<UChildActorComponent> m_ParentComponent;

		/** Array of ActorComponents that have been added by the user on a per-instance basis. */
		// TArray<TObjectPtr<UActorComponent>>
		KarmaVector<std::shared_ptr<UActorComponent>> m_InstanceComponents;

		/** Enum defining if BeginPlay has started or finished */
		enum class EActorBeginPlayState : uint8_t
		{
			HasNotBegunPlay,
			BeginningPlay,
			HasBegunPlay,
		};

		/**
		 * Indicates that BeginPlay has been called for this Actor.
		 * Set back to HasNotBegunPlay once EndPlay has been called.
		 */
		EActorBeginPlayState m_ActorHasBegunPlay : 2;

		/** 
		 * Set when DispatchBeginPlay() triggers from level streaming, and cleared afterwards.
		 * 
		 * @see IsActorBeginningPlayFromLevelStreaming().
		 */
		uint8_t m_bActorBeginningPlayFromLevelStreaming : 1;

		/** 
		 * Set if an Actor tries to be destroyed while it is beginning play so that once BeginPlay ends we can issue the destroy call.
		 */
		uint8_t m_bActorWantsDestroyDuringBeginPlay : 1;

		/**
		 * If true then destroy self when "finished", meaning all relevant components report that they are done and no timelines or timers are in flight. 
		 */
		uint8_t m_bAutoDestroyWhenFinished : 1;

	public:
		/** Return the ULevel that this Actor is part of. */
		ULevel* GetLevel() const;

		/** Called after the actor is spawned in the world.  Responsible for setting up actor for play. */
		void PostSpawnInitialize(FTransform const& SpawnTransform, AActor* InOwner, APawn* InInstigator, bool bRemoteOwned, bool bNoFail, bool bDeferConstruction);

		/** Getter for the cached world pointer, will return null if the actor is not actually spawned in a level */
		virtual UWorld* GetWorld() const override final;

		/** Returns whether an actor has had BeginPlay called on it (and not subsequently had EndPlay called) */
		bool HasActorBegunPlay() const { return m_ActorHasBegunPlay == EActorBeginPlayState::HasBegunPlay; }

		/** Returns whether an actor is in the process of beginning play */
		bool IsActorBeginningPlay() const { return m_ActorHasBegunPlay == EActorBeginPlayState::BeginningPlay; }

		/** Initiate a begin play call on this Actor, will handle calling in the correct order. */
		void DispatchBeginPlay(bool bFromLevelStreaming = false);

		/**
		 * Set the owner of this Actor, used primarily for network replication.
		 * @param NewOwner	The Actor who takes over ownership of this Actor
		 */
		virtual void SetOwner(AActor* NewOwner);

		/** Called to finish the spawning process, generally in the case of deferred spawning */
		void FinishSpawning(const FTransform& Transform, bool bIsDefaultTransform = false/*, const FComponentInstanceDataCache* InstanceDataCache = nullptr*/);

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

		/** Called after the actor has run its construction. Responsible for finishing the actor spawn process. */
		void PostActorConstruction();

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

		void GetComponents(KarmaVector<UActorComponent*>& OutComponents) const // make use of smartpointer ?
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

		/** Util to call OnComponentCreated on components */
		static void DispatchOnComponentsCreated(AActor* NewActor);

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
			return (m_RootComponent ? m_RootComponent->GetComponentTransform() : FTransform::m_Identity);
		}

		/** 
		 * Iterate over components array and call InitializeComponent, which happens once per actor 
		 * 
		 * @see AActor::PostActorConstruction()
		 */
		void InitializeComponents();

		/**
		 * Allow actors to initialize themselves on the C++ side after all of their components 
		 * have been initialized, only called during gameplay 
		 * 
		 * @see AActor::PostActorConstruction()
		 */
		virtual void PostInitializeComponents();

		/** 
		 * If this Actor was created by a Child Actor Component returns the Actor that owns that Child Actor Component  
		 * 
		 * @see AActor::PostActorConstruction()
		 */
		AActor* GetParentActor() const;

		/** If this Actor was created by a Child Actor Component returns that Child Actor Component  */
		UChildActorComponent* GetParentComponent() const;

		/**
		 * Removes a component from the OwnedComponents array of the Actor.
		 * In general this should not need to be called directly by anything other than UActorComponent functions
		 */
		void RemoveOwnedComponent(std::shared_ptr<UActorComponent> Component);

		bool GetAutoDestroyWhenFinished() const { return m_bAutoDestroyWhenFinished; }

		static USceneComponent* FixupNativeActorComponents(AActor* Actor);

		bool CanTick() const { return m_bCanEverTick; }
		void DisableTick(bool bDisable) { m_bCanEverTick = !bDisable; }

		/**
		 *	Function called every frame on this Actor. Override this function to implement custom logic to be executed every frame.
		 *	Note that Tick is enabled by default, and you will need to check m_bCanEverTick is set to false to disable it.
		 *
		 *	@param	DeltaSeconds	Game time elapsed during last frame modified by the time dilation
		 */
		virtual void Tick(float DeltaSeconds);

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
		 * Overridable native event for when play begins for this actor.
		 */
		virtual void BeginPlay();

		/** 
		 * Event when play begins for this actor.
		 * For blueprint trigger
		 */
		//void ReceiveBeginPlay();

	protected:
		/** 
		 * The component that defines the transform (location, rotation, scale) of this Actor 
		 * in the world, all other components must be attached to this one somehow.
		 * 
		 * Needs be raw pointer only
		 */
		USceneComponent* m_RootComponent;

		bool m_bCanEverTick;
	};
}
