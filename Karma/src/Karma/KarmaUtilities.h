#pragma once

#include "krpch.h"
#include "SubClassOf.h"
#include "Actor.h"
#include "stb_image.h"

namespace Karma
{
	class UWorld;
	class UObject;
	class AActor;
	class UClass;

	class KARMA_API KarmaUtilities
	{
	public:
		static std::string ReadFileToSpitString(const std::string& filePath);
		static std::string GetFilePath(const std::string& str);
		static unsigned char* GetImagePixelData(char const* fileName, int* width, int* height, int* channels, int req_comp);
	};

	/**
	 * Abstract base class for actor iteration. Implements all operators and relies on IsActorSuitable
	 * to be overridden by derived class.
	 * Note that when Playing In Editor, this will find actors only in CurrentWorld.
	 */
	class FActorIteratorState
	{
	public:
		/** Current world we are iterating upon                     */
		const UWorld* m_CurrentWorld;

		/** Results from the GetObjectsOfClass query                */
		KarmaVector<UObject*> m_ObjectArray;

		/** index of the current element in the object array        */
		int32_t m_Index;

		/** Whether we already reached the end                      */
		bool    m_ReachedEnd;

		/** Number of actors that have been considered thus far     */
		int32_t       m_ConsideredCount;

		/** Current actor pointed to by actor iterator              */
		AActor* m_CurrentActor;

		/** Contains any actors spawned during iteration            */
		KarmaVector<AActor*> m_SpawnedActorArray;

		/** The class type we are iterating, kept for filtering     */
		UClass* m_DesiredClass;

		/** Handle to the registered OnActorSpawned delegate        */
		//FDelegateHandle ActorSpawnedDelegateHandle;

		/**
		* Default constructor, initializes everything relevant
		*/
		FActorIteratorState(const UWorld* InWorld, const TSubclassOf<AActor> InClass) :
			m_CurrentWorld(InWorld),
			m_Index(-1),
			m_ReachedEnd(false),
			m_ConsideredCount(0),
			m_CurrentActor(nullptr),
			m_DesiredClass(InClass)
		{
			//check(IsInGameThread());
			KR_CORE_ASSERT(m_CurrentWorld, "");

	#if WITH_EDITOR
			// In the editor, you are more likely to have many worlds in memory at once.
			// As an optimization to avoid iterating over many actors that are not in the world we are asking for,
			// if the filter class is AActor, just use the actors that are in the world you asked for.
			// This could be useful in runtime code as well if there are many worlds in memory, but for now we will leave
			// it in editor code.
			if (InClass == AActor::StaticClass())
			{
				// First determine the number of actors in the world to reduce reallocations when we append them to the array below.
				int32 NumActors = 0;
				for (ULevel* Level : InWorld->GetLevels())
				{
					if (Level)
					{
						NumActors += Level->Actors.Num();
					}
				}


				// Presize the array
				ObjectArray.Reserve(NumActors);


				// Fill the array
				for (ULevel* Level : InWorld->GetLevels())
				{
					if (Level)
					{
						ObjectArray.Append(Level->Actors);
					}
				}
			}
			else
	#endif // WITH_EDITOR
			{
				constexpr EObjectFlags ExcludeFlags = RF_ClassDefaultObject;
				GetObjectsOfClass(InClass, m_ObjectArray, true, ExcludeFlags, EInternalObjectFlags::Garbage);
			}


			// const auto ActorSpawnedDelegate = FOnActorSpawned::FDelegate::CreateRaw(this, &FActorIteratorState::OnActorSpawned);
			// m_ActorSpawnedDelegateHandle = m_CurrentWorld->AddOnActorSpawnedHandler(ActorSpawnedDelegate);
		}


		~FActorIteratorState()
		{
			//m_CurrentWorld->RemoveOnActorSpawnedHandler(ActorSpawnedDelegateHandle);
		}


		/**
		* Returns the current suitable actor pointed at by the Iterator
		*
		* @return  Current suitable actor
		*/
		FORCEINLINE AActor* GetActorChecked() const
		{
			KR_CORE_ASSERT(m_CurrentActor, "");
			KR_CORE_ASSERT(!m_CurrentActor->IsUnreachable(), "");

			return m_CurrentActor;
		}

	private:
		/*
		void OnActorSpawned(AActor* InActor)
		{
			if (InActor->IsA(DesiredClass))
			{
				SpawnedActorArray.AddUnique(InActor);
			}
		}*/
	};

	/** Iteration flags, specifies which types of levels and actors should be iterated */
	enum class EActorIteratorFlags
	{
		AllActors			= 0x00000000, // No flags, iterate all actors
		SkipPendingKill		= 0x00000001, // Skip pending kill actors
		OnlySelectedActors	= 0x00000002, // Only iterate actors that are selected
		OnlyActiveLevels	= 0x00000004, // Only iterate active levels
	};

	/**
	 *  Template class used to filter actors by certain characteristics
	 */
	class TActorIteratorBase
	{
	private:
		EActorIteratorFlags m_Flags;

	public:
		/**
		 * Iterates to next suitable actor.
		 */
		void operator++()
		{
			// Use local version to avoid LHSs
			AActor* LocalCurrentActor = nullptr;
			int32_t LocalIndex;
		}
	};
}
