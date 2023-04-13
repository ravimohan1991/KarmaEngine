#include "Actor.h"
#include "GameFramework/Level.h"
#include "World.h"
#include "GameFramework/Pawn.h"
#include "Ganit/Transform.h"
#include "ChildActorComponent.h"

namespace Karma
{
	uint32_t AActor::m_BeginPlayCallDepth = 0;

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

	void AActor::DispatchOnComponentsCreated(AActor* NewActor)
	{
		KarmaVector<UActorComponent*> Components;
		NewActor->GetComponents(Components);

		for (UActorComponent* ActorComp : Components)
		{
			if (!ActorComp->HasBeenCreated())
			{
				ActorComp->OnComponentCreated();
			}
		}
	}

	void AActor::FinishSpawning(const FTransform& UserTransform, bool bIsDefaultTransform/*, const FComponentInstanceDataCache* InstanceDataCache*/)
	{
		if (!m_bHasFinishedSpawning)
		{
			m_bHasFinishedSpawning = true;

			FTransform FinalRootComponentTransform = (m_RootComponent ? m_RootComponent->GetComponentTransform() : UserTransform);

			// see if we need to adjust the transform (i.e. in deferred cases where the caller passes in a different transform here 
			// than was passed in during the original SpawnActor call)
			if (m_RootComponent && !bIsDefaultTransform)
			{
				//FTransform const* const OriginalSpawnTransform = GSpawnActorDeferredTransformCache.Find(this);
				/*if (OriginalSpawnTransform)
				{
					GSpawnActorDeferredTransformCache.Remove(this);

					if (OriginalSpawnTransform->Equals(UserTransform) == false)
					{
						UserTransform.GetLocation().DiagnosticCheckNaN(TEXT("AActor::FinishSpawning: UserTransform.GetLocation()"));
						UserTransform.GetRotation().DiagnosticCheckNaN(TEXT("AActor::FinishSpawning: UserTransform.GetRotation()"));

						// caller passed a different transform!
						// undo the original spawn transform to get back to the template transform, so we can recompute a good
						// final transform that takes into account the template's transform
						FTransform const TemplateTransform = RootComponent->GetComponentTransform() * OriginalSpawnTransform->Inverse();
						FinalRootComponentTransform = TemplateTransform * UserTransform;
					}
				}*/

				// should be fast and relatively rare
				//ValidateDeferredTransformCache();
			}


			{
				//FEditorScriptExecutionGuard ScriptGuard;
				//ExecuteConstruction(FinalRootComponentTransform, nullptr, InstanceDataCache, bIsDefaultTransform);
			}

			{
				//SCOPE_CYCLE_COUNTER(STAT_PostActorConstruction);
				PostActorConstruction();
			}
		}
	}

	void AActor::InitializeComponents()
	{
		//QUICK_SCOPE_CYCLE_COUNTER(STAT_Actor_InitializeComponents);

		KarmaVector<UActorComponent*> Components;
		GetComponents(Components);

		for (UActorComponent* ActorComp : Components)
		{
			if (ActorComp->IsRegistered())
			{
				if (ActorComp->m_bAutoActivate && !ActorComp->IsActive())
				{
					ActorComp->Activate(true);
				}

				if (ActorComp->m_bWantsInitializeComponent && !ActorComp->HasBeenInitialized())
				{
					// Broadcast the activation event since Activate occurs too early to fire a callback in a game
					ActorComp->InitializeComponent();
				}
			}
		}
	}

	void AActor::PostInitializeComponents()
	{
		//QUICK_SCOPE_CYCLE_COUNTER(STAT_Actor_PostInitComponents);

		if (IsValidChecked(this))
		{
			m_bActorInitialized = true;

			//UpdateAllReplicatedComponents();
		}
	}

	AActor* AActor::GetParentActor() const
	{
		AActor* ParentActor = nullptr;
		if (UChildActorComponent* ParentComponentPtr = GetParentComponent())
		{
			ParentActor = ParentComponentPtr->GetOwner();
		}

		return ParentActor;
	}

	void AActor::RemoveOwnedComponent(std::shared_ptr<UActorComponent> Component)
	{
		// Note: we do not mark dirty here because this can be called as part of component duplication when reinstancing components during blueprint compilation
		// if a component is removed during this time it should not dirty.  Higher level code in the editor should always dirty the package anyway.
		const bool bMarkDirty = false;

		// Garbage collection
		// Modify(bMarkDirty);

		if (m_OwnedComponents.Remove(Component) > 0)
		{
			/*
			ReplicatedComponents.RemoveSingleSwap(Component);
			RemoveReplicatedComponent(Component);
			*/
			/*
			if (Component->IsCreatedByConstructionScript())
			{
				BlueprintCreatedComponents.RemoveSingleSwap(Component);
			}
			else if (Component->m_CreationMethod == EComponentCreationMethod::Instance)
			{
				m_InstanceComponents.RemoveSingleSwap(Component);
			}
			*/
		}
	}

	UChildActorComponent* AActor::GetParentComponent() const
	{
		return m_ParentComponent.get();
	}

	void AActor::PostActorConstruction()
	{
		UWorld* const World = GetWorld();
		bool const bActorsInitialized = World && World->AreActorsInitialized();

		if (bActorsInitialized)
		{
			// To be written once rigged with input system
			//PreInitializeComponents();
		}

		// If this is dynamically spawned replicated actor, defer calls to BeginPlay and UpdateOverlaps until replicated properties are deserialized
		//const bool bDeferBeginPlayAndUpdateOverlaps = (bExchangedRoles && RemoteRole == ROLE_Authority) && !GIsReinstancing;

		if (bActorsInitialized)
		{
			// Call InitializeComponent on components
			InitializeComponents();

			// actor should have all of its components created and registered now, do any collision checking and handling that we need to do
			if (World)
			{
				/*
				switch (SpawnCollisionHandlingMethod)
				{
				case ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn:
				{
					// Try to find a spawn position
					FVector AdjustedLocation = GetActorLocation();
					FRotator AdjustedRotation = GetActorRotation();
					if (World->FindTeleportSpot(this, AdjustedLocation, AdjustedRotation))
					{
						SetActorLocationAndRotation(AdjustedLocation, AdjustedRotation, false, nullptr, ETeleportType::TeleportPhysics);
					}
				}
				break;
				case ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding:
				{
					// Try to find a spawn position			
					FVector AdjustedLocation = GetActorLocation();
					FRotator AdjustedRotation = GetActorRotation();
					if (World->FindTeleportSpot(this, AdjustedLocation, AdjustedRotation))
					{
						SetActorLocationAndRotation(AdjustedLocation, AdjustedRotation, false, nullptr, ETeleportType::TeleportPhysics);
					}
					else
					{
						UE_LOG(LogSpawn, Warning, TEXT("SpawnActor failed because of collision at the spawn location [%s] for [%s]"), *AdjustedLocation.ToString(), *GetClass()->GetName());
						Destroy();
					}
				}
				break;
				case ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding:
					if (World->EncroachingBlockingGeometry(this, GetActorLocation(), GetActorRotation()))
					{
						UE_LOG(LogSpawn, Warning, TEXT("SpawnActor failed because of collision at the spawn location [%s] for [%s]"), *GetActorLocation().ToString(), *GetClass()->GetName());
						Destroy();
					}
					break;
				case ESpawnActorCollisionHandlingMethod::Undefined:
				case ESpawnActorCollisionHandlingMethod::AlwaysSpawn:
				default:
					// note we use "always spawn" as default, so treat undefined as that
					// nothing to do here, just proceed as normal
					break;
				}*/
			}

			if (IsValidChecked(this))
			{
				PostInitializeComponents();
				if (IsValidChecked(this))
				{
					if (!m_bActorInitialized)
					{
						KR_CORE_ASSERT(false, "{0} failed to route PostInitializeComponents.  Please call Super::PostInitializeComponents() in your <className>::PostInitializeComponents() function", GetName());
						//UE_LOG(LogActor, Fatal, TEXT("%s failed to route PostInitializeComponents.  Please call Super::PostInitializeComponents() in your <className>::PostInitializeComponents() function. "), *GetFullName());
					}

					bool bRunBeginPlay = /*!bDeferBeginPlayAndUpdateOverlaps &&*/ (m_BeginPlayCallDepth > 0 || World->HasBegunPlay());
					if (bRunBeginPlay)
					{
						if (AActor* ParentActor = GetParentActor())
						{
							// Child Actors cannot run begin play until their parent has run
							bRunBeginPlay = (ParentActor->HasActorBegunPlay() || ParentActor->IsActorBeginningPlay());
						}
					}
/*
#if WITH_EDITOR
					if (bRunBeginPlay && bIsEditorPreviewActor)
					{
						bRunBeginPlay = false;
					}
#endif
*/
					if (bRunBeginPlay)
					{
						//SCOPE_CYCLE_COUNTER(STAT_ActorBeginPlay);
						DispatchBeginPlay();
					}
				}
			}
		}
		else
		{
			// Invalidate the object so that when the initial undo record is made,
			// the actor will be treated as destroyed, in that undo an add will
			// actually work
			MarkAsGarbage();
			
			// If with editor
			// Modify(false);
			ClearGarbage();
		}
	}

	void AActor::DispatchBeginPlay(bool bFromLevelStreaming)
	{
		UWorld* World = (!HasActorBegunPlay() && IsValidChecked(this) ? GetWorld() : nullptr);

		if (World)
		{
			KR_CORE_ASSERT(m_ActorHasBegunPlay == EActorBeginPlayState::HasNotBegunPlay, "BeginPlay was called on actor {0} which was in state {1}", /**GetPathName()*/GetName(), (int32)m_ActorHasBegunPlay);
			const uint32_t CurrentCallDepth = m_BeginPlayCallDepth++;

			m_bActorBeginningPlayFromLevelStreaming = bFromLevelStreaming;
			m_ActorHasBegunPlay = EActorBeginPlayState::BeginningPlay;

			//BuildReplicatedComponentsInfo();

/*
#if UE_WITH_IRIS
			BeginReplication();
#endif // UE_WITH_IRIS
*/

			BeginPlay();

			KR_CORE_ASSERT(m_BeginPlayCallDepth - 1 == CurrentCallDepth, "");
			m_BeginPlayCallDepth = CurrentCallDepth;

			if (m_bActorWantsDestroyDuringBeginPlay)
			{
				// Pass true for bNetForce as either it doesn't matter or it was true the first time to even 
				// get to the point we set bActorWantsDestroyDuringBeginPlay to true
				World->ShivaActor(this, true);
			}

			if (IsValidChecked(this))
			{
				// Initialize overlap state. Will be functional once physics is written
				// UpdateInitialOverlaps(bFromLevelStreaming);
			}

			m_bActorBeginningPlayFromLevelStreaming = false;
		}
	}

	void AActor::BeginPlay()
	{
		// Find the purpose of this macro
		// TRACE_OBJECT_LIFETIME_BEGIN(this);

		KR_CORE_ASSERT(m_ActorHasBegunPlay == EActorBeginPlayState::BeginningPlay, "BeginPlay was called on actor {0} which was in state {1}", GetName(), (int32)m_ActorHasBegunPlay);
		
		// Need to write timer routines
		//SetLifeSpan(InitialLifeSpan);
		//RegisterAllActorTickFunctions(true, false); // Components are done below.

		KarmaVector<UActorComponent*> Components;
		GetComponents(Components);

		for (UActorComponent* Component : Components)
		{
			// bHasBegunPlay will be true for the component if the component was renamed and moved to a new outer during initialization
			if (Component->IsRegistered() && !Component->HasBegunPlay())
			{
				//Component->RegisterAllComponentTickFunctions(true);
				Component->BeginPlay();
				KR_CORE_ASSERT(Component->HasBegunPlay(), "Failed to route BeginPlay ({0})", Component->GetName());
			}
			else
			{
				// When an Actor begins play we expect only the not bAutoRegister false components to not be registered
				//check(!Component->bAutoRegister);
			}
		}

		if (GetAutoDestroyWhenFinished())
		{
			if (UWorld* MyWorld = GetWorld())
			{
				// TODO: write AutoDestorySys for which we need to write SubSystem class
				/*
				if (UAutoDestroySubsystem* AutoDestroySys = MyWorld->GetSubsystem<UAutoDestroySubsystem>())
				{
					AutoDestroySys->RegisterActor(this);
				}
				*/
			}
		}

		// For blueprint trigger
		//ReceiveBeginPlay();
		m_ActorHasBegunPlay = EActorBeginPlayState::HasBegunPlay;
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

		m_CreationTime = (World ? (float)World->GetTimeSeconds() : 0.f);

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
		//bHasDeferredComponentRegistration = (SceneRootComponent == nullptr && Cast<UBlueprintGeneratedClass>(GetClass()) != nullptr);
		//if (!bHasDeferredComponentRegistration && GetWorld())
		//{
		//	RegisterAllComponents();
		//}

		// See if anything has deleted us
		if(!IsValidChecked(this) && !bNoFail)
		{
			return;
		}

		// Send messages. We've fully spawned
		// PostActorCreated();// nothing at the moment

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
			// GSpawnActorDeferredTransformCache.Emplace(this, UserSpawnTransform);
		}
	}

	void AActor::SetInstigator(APawn* InInstigator)
	{
		m_Instigator = InInstigator;

		//MARK_PROPERTY_DIRTY_FROM_NAME(AActor, Instigator, this);
	}

	/** Util that sets up the actor's component hierarchy (when users forget to do so, in their native ctor) */
	USceneComponent* AActor::FixupNativeActorComponents(AActor* Actor)
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
