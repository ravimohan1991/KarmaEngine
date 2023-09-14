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
			m_GameInstance = NewObject<UGameInstance>(this, UGameInstance::StaticClass(), "EkDa");
			m_GameInstance->InitializeStandalone("AWholeNewWorld");
		}
	}

	void KEngine::Tick(float DeltaSeconds, bool bIdle)
	{
		// Tick the worlds
		for (uint32_t WorldIdx = 0; WorldIdx < m_WorldList.Num(); ++WorldIdx)
		{
			FWorldContext& Context = *m_WorldList.GetElements()[WorldIdx];

			if (Context.World() == nullptr /* || !Context.World()->ShouldTick()*/)
			{
				continue;
			}

			UWorld* aWorld = Context.World();

			// Tick all travel and Pending NetGames (Seamless, server, client)

			// Actual worldtick
			{
				aWorld->Tick(DeltaSeconds);
			}
		}
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
