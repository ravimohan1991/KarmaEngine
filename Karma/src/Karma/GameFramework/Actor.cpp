#include "Actor.h"
#include "GameFramework/Level.h"
#include "World.h"
#include "GameFramework/Pawn.h"
#include "Ganit/Transform.h"

namespace Karma
{
	AActor::AActor()
	{
		m_Owner = nullptr;
		m_RootComponent = nullptr;
	}

	ULevel* AActor::GetLevel() const
	{
		return GetTypedOuter<ULevel>();
	}

	UWorld* AActor::GetWorld() const
	{
		KR_CORE_ASSERT(GetOuter(), "Actor: {0} has a null OuterPrivate in 	AActor::GetWorld()");// , *GetFullName())

		// CDO objects do not belong to a world
		// If the actors outer is destroyed or unreachable we are shutting down and the world should be nullptr
		if (!HasAnyFlags(RF_ClassDefaultObject) && !GetOuter()->HasAnyFlags(RF_BeginDestroyed))// && !GetOuter()->IsUnreachable())
		{
			if (ULevel* Level = GetLevel())
			{
				return Level->m_OwningWorld;
			}
		}
		return nullptr;
	}

	void AActor::SetOwner(AActor* NewOwner)
	{
		// If NewOwner is different and Actor not marked for GC
		if (m_Owner != NewOwner && IsValidChecked(this))
		{
			if (NewOwner != nullptr && NewOwner->IsOwnedBy(this))
			{
				KR_CORE_ERROR("SetOwner(): Failed to set {0} owner of {1} because it would cause an Owner loop", NewOwner->GetName(), GetName());
				return;
			}

			// Sets this actor's parent to the specified actor.
			if (m_Owner != nullptr)
			{
				// remove from old owner's Children array
				KR_CORE_ASSERT(m_Owner->m_Children.Remove(this) == 1, "More than one occurences of this actor {0} in children of owner {1}", this->GetName(), m_Owner->GetName());
			}

			m_Owner = NewOwner;

			//MARK_PROPERTY_DIRTY_FROM_NAME(AActor, Owner, this);

			if (m_Owner != nullptr)
			{
				// add to new owner's Children array

				// Check if owner does not already have this as child
				KR_CORE_ASSERT(!m_Owner->m_Children.Contains(this), "Owner already has this as child");
				
				m_Owner->m_Children.Add(this);
			}

			// mark all components for which Owner is relevant for visibility to be updated
			/*
			if (bHasFinishedSpawning)
			{
				MarkOwnerRelevantComponentsDirty(this);
			}*/
		}
	}

	void AActor::PostSpawnInitialize(FTransform const& UserSpawnTransform, AActor* InOwner, APawn* InInstigator, bool bRemoteOwned, bool bNoFail, bool bDeferConstruction)
	{
		// General flow here is like so
		// - Actor sets up the basics.
		// - Actor gets PreInitializeComponents()
		// - Actor constructs itself, after which its components should be fully assembled
		// - Actor components get OnComponentCreated
		// - Actor components get InitializeComponent
		// - Actor gets PostInitializeComponents() once everything is set up
		//
		// This should be the same sequence for deferred or nondeferred spawning.

		// It's not safe to call UWorld accessor functions till the world info has been spawned.
		UWorld* const World = GetWorld();
		bool const bActorsInitialized = World && World->AreActorsInitialized();

		m_CreationTime = (World ? World->GetTimeSeconds() : 0.f);

		// Set owner.
		SetOwner(InOwner);

		// Set instigator
		SetInstigator(InInstigator);

		// Set the actor's world transform if it has a native rootcomponent.
		USceneComponent* const SceneRootComponent = FixupNativeActorComponents(this);
		if (SceneRootComponent != nullptr)
		{
			KR_CORE_ASSERT(SceneRootComponent->GetOwner() == this, "Owner is not this {0}", this->GetName());

			// Respect any non-default transform value that the root component may have received from the archetype that's owned
			// by the native CDO, so the final transform might not always necessarily equate to the passed-in UserSpawnTransform.
			const FTransform RootTransform(SceneRootComponent->GetRelativeRotation(), SceneRootComponent->GetRelativeLocation(), SceneRootComponent->GetRelativeScale3D());
			const FTransform FinalRootComponentTransform = RootTransform * UserSpawnTransform;

			SceneRootComponent->SetWorldTransform(FinalRootComponentTransform/*, false, nullptr, ETeleportType::ResetPhysics*/);
		}

		// Call OnComponentCreated on all default (native) components
		DispatchOnComponentsCreated(this);

		// Register the actor's default (native) components, but only if we have a native scene root. If we don't, it implies that there could be only non-scene components
		// at the native class level. In that case, if this is a Blueprint instance, we need to defer native registration until after SCS execution can establish a scene root.
		// Note: This API will also call PostRegisterAllComponents() on the actor instance. If deferred, PostRegisterAllComponents() won't be called until the root is set by SCS.
		bHasDeferredComponentRegistration = (SceneRootComponent == nullptr && Cast<UBlueprintGeneratedClass>(GetClass()) != nullptr);
		if (!bHasDeferredComponentRegistration && GetWorld())
		{
			RegisterAllComponents();
		}

		// See if anything has deleted us
		if( !IsValidChecked(this) && !bNoFail )
		{
			return;
		}

		// Send messages. We've fully spawned
		PostActorCreated();

		// Executes native and BP construction scripts.
		// After this, we can assume all components are created and assembled.
		if (!bDeferConstruction)
		{
			FinishSpawning(UserSpawnTransform, true);
		}
		else if (SceneRootComponent != nullptr)
		{
			// we have a native root component and are deferring construction, store our original UserSpawnTransform
			// so we can do the proper thing if the user passes in a different transform during FinishSpawning
			GSpawnActorDeferredTransformCache.Emplace(this, UserSpawnTransform);
		}
	}

	void AActor::SetInstigator(APawn* InInstigator)
	{
		m_Instigator = InInstigator;

		//MARK_PROPERTY_DIRTY_FROM_NAME(AActor, Instigator, this);
	}

	/** Util that sets up the actor's component hierarchy (when users forget to do so, in their native ctor) */
	static USceneComponent* FixupNativeActorComponents(AActor* Actor)
	{
		USceneComponent* SceneRootComponent = Actor->GetRootComponent();
		if (SceneRootComponent == nullptr)
		{
			//TInlineComponentArray<USceneComponent*> SceneComponents;
			KarmaVector<USceneComponent*> SceneComponents;
			Actor->GetComponents(SceneComponents);

			if (SceneComponents.Num() > 0)
			{
				KR_CORE_INFO("{0} has natively added scene component(s), but none of them were set as the actor's RootComponent - picking one arbitrarily", Actor->GetName());

				// if the user forgot to set one of their native components as the root, 
				// we arbitrarily pick one for them (otherwise the SCS could attempt to 
				// create its own root, and nest native components under it)
				for (USceneComponent* Component : SceneComponents)
				{
					if ((Component == nullptr) ||
						(Component->GetAttachParent() != nullptr) ||
						(Component->m_CreationMethod != EComponentCreationMethod::Native))
					{
						continue;
					}

					SceneRootComponent = Component;
					Actor->SetRootComponent(Component);
					break;
				}
			}
		}

		return SceneRootComponent;
	}

	bool AActor::SetRootComponent(class USceneComponent* NewRootComponent)
	{
		/** Only components owned by this actor can be used as a its root component. */
		// Hmm why newrootcomponent be null
		if (NewRootComponent == nullptr || NewRootComponent->GetOwner() == this)
		{
			if (m_RootComponent != NewRootComponent)
			{
				//Modify(); for editor purposes

				USceneComponent* OldRootComponent = m_RootComponent;
				m_RootComponent = NewRootComponent;

				// Notify new root first, as it probably has no delegate on it.
				if (NewRootComponent)
				{
					//NewRootComponent->NotifyIsRootComponentChanged(true);
				}

				if (OldRootComponent)
				{
					//OldRootComponent->NotifyIsRootComponentChanged(false);
				}
			}
			return true;
		}

		return false;
	}
}
