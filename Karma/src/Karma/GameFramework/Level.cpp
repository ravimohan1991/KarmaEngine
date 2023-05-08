#include "Level.h"
#include "WorldSettings.h"

namespace Karma
{
	ULevel::ULevel() : UObject()
	{
		
	}

	void ULevel::Initialize(const FURL& InURL)
	{
		//m_URL = InURL;
	}

	// Not completely functional yet
	void ULevel::SetWorldSettings(AWorldSettings* NewWorldSettings)
	{
		KR_CORE_ASSERT(NewWorldSettings, "No NewWorldSettings");

		if (m_WorldSettings != NewWorldSettings)
		{
			// We'll generally endeavor to keep the world settings at its traditional index 0
			/*const int32 NewWorldSettingsIndex = m_Actors.FindLast(NewWorldSettings);

			if (NewWorldSettingsIndex != 0)
			{
				if (Actors[0] == nullptr || Actors[0]->IsA<AWorldSettings>())
				{
					Exchange(Actors[0], Actors[NewWorldSettingsIndex]);
				}
				else
				{
					Actors[NewWorldSettingsIndex] = nullptr;
					Actors.Insert(NewWorldSettings, 0);
				}
			}*/

			// Assign the new world settings before destroying the old ones
			// since level will prevent destruction of the world settings if it matches the cached value
			m_WorldSettings = NewWorldSettings;

			// Makes no sense to have several WorldSettings so destroy existing ones
			/*for (int32 ActorIndex = 1; ActorIndex < Actors.Num(); ActorIndex++)
			{
				if (AActor* Actor = Actors[ActorIndex])
				{
					if (AWorldSettings* ExistingWorldSettings = Cast<AWorldSettings>(Actor))
					{
						check(ExistingWorldSettings != WorldSettings);
						ExistingWorldSettings->Destroy();
					}
				}
			}*/
		}
	}
}
