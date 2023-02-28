#include "ActorComponent.h"

namespace Karma
{
	void UActorComponent::TickComponent(float deltaTime)
	{
	
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

	void UActorComponent::EndPlay()
	{
		KR_CORE_ASSERT(m_bHasBegunPlay, "Calling EndPlay() without calling BeginPlay() first.");

		m_bHasBegunPlay = false;
	}
}