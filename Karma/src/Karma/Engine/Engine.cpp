#include "Engine.h"
#include "GameInstance.h"

#include "Core/UObjectGlobals.h"

namespace Karma
{
	KEngine*	GEngine;

	void FWorldContext::SetCurrentWorld(UWorld* World)
	{
		m_ThisCurrentWorld = World;
	}

	KEngine::KEngine()
	{
		KR_CORE_INFO("Initializing KEngine");
		Init();
	}

	void KEngine::Init()
	{
		/*UWorld *aWorld = UWorld::CreateWorld(EWorldType::Game, true, "AWholeNewWorld");

		FWorldContext aWorldContext = CreateNewWorldContext(EWorldType::Game);
		aWorldContext.SetCurrentWorld(aWorld);*/

		// Spawning GameInstance
		{
			m_GameInstance = NewObject<UGameInstance>(this, UGameInstance::StaticClass());
			m_GameInstance->InitializeStandalone();
		}
	}

	void KEngine::Tick(float DeltaSeconds, bool bIdle)
	{
	}

	FWorldContext& KEngine::CreateNewWorldContext(EWorldType::Type WorldType)
	{
		// Need to see how UE does this
		FWorldContext* aWorldContext = new FWorldContext();
		m_WorldList.Add(aWorldContext);

		return *m_WorldList.IndexToObject(m_WorldList.Num() - 1);
	}

	void KEngine::CleanupGameViewport()
	{
		// Clean up
	}
}
