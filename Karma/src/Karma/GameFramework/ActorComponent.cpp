#include "ActorComponent.h"
#include "KarmaTypes.h"

#include "Actor.h"

namespace Karma
{
	void UActorComponent::TickComponent(float deltaTime)
	{
	
	}

	void UActorComponent::InitializeComponent()
	{
		KR_CORE_ASSERT(m_bRegistered, "Component not registered");
		KR_CORE_ASSERT(!m_bHasBeenInitialized, "Component already initialized");

		m_bHasBeenInitialized = true;
	}

	void UActorComponent::BeginPlay()
	{
		// Introduce checks for RegisterAllComponentTickFunctions().
		/*
		check(bRegistered); KR_CORE_ASSERT();
		check(!bHasBegunPlay);
		checkSlow(bTickFunctionsRegistered); // If this fails, someone called BeginPlay() without first calling RegisterAllComponentTickFunctions().
		*/
		
		m_bHasBegunPlay = true;
	}

	void UActorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
	{
		KR_CORE_ASSERT(m_bHasBegunPlay, "Calling EndPlay() without calling BeginPlay() first.");

		// If we're in the process of being garbage collected it is unsafe to call out to blueprints
		/*if (!HasAnyFlags(RF_BeginDestroyed) && !IsUnreachable() && (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native)))
		{
			ReceiveEndPlay(EndPlayReason);
		}*/

		m_bHasBegunPlay = false;
	}

	void UActorComponent::OnComponentCreated()
	{
		//ensure(!bHasBeenCreated);
		KR_CORE_ASSERT(!m_bHasBeenCreated, "Already called this routine");
		m_bHasBeenCreated = true;
	}

	AActor* UActorComponent::GetOwner() const
	{
		//checkSlow(OwnerPrivate == GetActorOwnerNoninline()); // verify cached value is correct
		return m_OwnerPrivate;
	}

	void UActorComponent::Activate(bool bReset)
	{
		if (bReset || ShouldActivate() == true)
		{
			//SetComponentTickEnabled(true);
			SetActiveFlag(true);

			//OnComponentActivated.Broadcast(this, bReset);
		}
	}

	void UActorComponent::SetActiveFlag(const bool bNewIsActive)
	{
		m_bIsActive = bNewIsActive;
		//MARK_PROPERTY_DIRTY_FROM_NAME(UActorComponent, bIsActive, this);
	}

	bool UActorComponent::ShouldActivate() const
	{
		// if not active, should activate
		return !IsActive();
	}

	void UActorComponent::UninitializeComponent()
	{
		KR_CORE_ASSERT(m_bHasBeenInitialized, "ActorComponent already uninitialized");

		m_bHasBeenInitialized = false;
	}

	void UActorComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
	{
		// @TODO: Would be nice to ensure(bHasBeenCreated), but there are still many places where components are created without calling OnComponentCreated
		m_bHasBeenCreated = false;
	}

	void UActorComponent::BeginDestroy()
	{
		if (m_bHasBegunPlay)
		{
			EndPlay(EEndPlayReason::Destroyed);
		}

		// Ensure that we call UninitializeComponent before we destroy this component
		if (m_bHasBeenInitialized)
		{
			UninitializeComponent();
		}

		//bIsReadyForReplication = false;

		//ExecuteUnregisterEvents();

		// Ensure that we call OnComponentDestroyed before we destroy this component
		if (m_bHasBeenCreated)
		{
			OnComponentDestroyed(/*GExitPurge*/ false);
		}

		m_WorldPrivate = nullptr;

		std::shared_ptr<UActorComponent> smartThis;
		smartThis.reset(this);

		// Remove from the parent's OwnedComponents list
		if (AActor* MyOwner = GetOwner())
		{
			MyOwner->RemoveOwnedComponent(smartThis);
		}
/*
#if WITH_EDITOR
		UEngineElementsLibrary::DestroyEditorComponentElement(this);
#endif	// WITH_EDITOR
*/
		//ClearUCSModifiedProperties();

		UObject::BeginDestroy();
	}
}