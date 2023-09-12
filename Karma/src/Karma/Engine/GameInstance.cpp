#include "GameInstance.h"
#include "Core/Package.h"
#include "GameFramework/World.h"
#include "Engine.h"

namespace Karma
{
	void UGameInstance::Init()
	{
		
	}

	KEngine* UGameInstance::GetEngine() const
	{
		return static_cast<KEngine*>(GetOuter());
	}

	void UGameInstance::InitializeStandalone(const std::string& InWorldName, UPackage* InWorldPackage)
	{
		// Creates the world context. This should be the only WorldContext that ever gets created for this GameInstance.
		WorldContext = &GetEngine()->CreateNewWorldContext(EWorldType::Game);
		WorldContext->m_OwningGameInstance = this;

		// In standalone create a dummy world from the beginning to avoid issues of not having a world until LoadMap gets us our 	real world
		UWorld* aWorld = UWorld::CreateWorld(EWorldType::Game, false, InWorldName, InWorldPackage);
		aWorld->SetGameInstance(this);
		WorldContext->SetCurrentWorld(aWorld);

		Init();
	}
}
