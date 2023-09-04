#pragma once

#include "krpch.h"
#include "SubClassOf.h"
#include "Actor.h"
#include "stb_image.h"
#include "Level.h"
#include "World.h"
#include "WorldSettings.h"

namespace Karma
{
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

		FActorIteratorState() {};

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
	enum EActorIteratorFlags
	{
		AllActors			= 0x00000000, // No flags, iterate all actors
		SkipPendingKill		= 0x00000001, // Skip pending kill actors
		OnlySelectedActors	= 0x00000002, // Only iterate actors that are selected
		OnlyActiveLevels	= 0x00000004, // Only iterate active levels
	};

	/** Type enum, used to represent the special End iterator */
	enum class EActorIteratorType
	{
		End
	};

	/**
	 *  Template class used to filter actors by certain characteristics
	 */
	template <typename Derived>
	class TActorIteratorBase
	{
	private:
		EActorIteratorFlags m_Flags;

		// smart pointer?
		std::shared_ptr<FActorIteratorState> m_State;

	protected:
		/**
		 * Hide the constructors as construction on this class should only be done by subclasses
		 */
		TActorIteratorBase(EActorIteratorType)
			: m_Flags(EActorIteratorFlags::AllActors)
		{
		}

		TActorIteratorBase(const UWorld* InWorld, TSubclassOf<AActor> InClass, const EActorIteratorFlags InFlags)
			: m_Flags(InFlags)
		{
			//m_State.Emplace(InWorld, InClass);

			m_State.reset(new FActorIteratorState(InWorld, InClass));
			//m_State->m_CurrentWorld = InWorld;
			//m_State->m_DesiredClass = InClass;
		}

	public:
		/**
		 * Iterates to next suitable actor.
		 */
		void operator++()
		{
			// Use local version to avoid LHSs as compiler is not required to write out member variables to memory.
			AActor* localCurrentActor = nullptr;
			int32_t             localIndex = m_State->m_Index;
			KarmaVector<UObject*>& localObjectArray = m_State->m_ObjectArray;
			KarmaVector<AActor*>& localSpawnedActorArray = m_State->m_SpawnedActorArray;
			const UWorld* localCurrentWorld = m_State->m_CurrentWorld;

			while (++localIndex < (localObjectArray.Num() + localSpawnedActorArray.Num()))
			{
				if (localIndex < localObjectArray.Num())
				{
					localCurrentActor = static_cast<AActor*>(localObjectArray.GetElements()[localIndex]);
				}
				else
				{
					localCurrentActor = localSpawnedActorArray.GetElements()[localIndex - localObjectArray.Num()];
				}
				m_State->m_ConsideredCount++;

				ULevel* actorLevel = localCurrentActor ? localCurrentActor->GetLevel() : nullptr;

				if (actorLevel
					&& static_cast<const Derived*>(this)->IsActorSuitable(localCurrentActor)
					&& static_cast<const Derived*>(this)->CanIterateLevel(actorLevel)
					/*&& actorLevel->GetWorld()->GetName() == localCurrentWorld->GetName()*/)
				{
					// ignore non-persistent world settings
					if (actorLevel == localCurrentWorld->GetPersistentLevel() || !localCurrentActor->IsA(AWorldSettings::StaticClass()))
					{
						m_State->m_CurrentActor = localCurrentActor;
						m_State->m_Index = localIndex;
						return;
					}
				}
			}
			m_State->m_CurrentActor = nullptr;
			m_State->m_ReachedEnd = true;
		}

		/**
		 * Returns the current suitable actor pointed at by the Iterator
		 *
		 * @return	Current suitable actor
		 */
		FORCEINLINE AActor* operator*() const
		{
			return m_State->GetActorChecked();
		}

		/**
		 * Returns the current suitable actor pointed at by the Iterator
		 *
		 * @return	Current suitable actor
		 */
		FORCEINLINE AActor* operator->() const
		{
			return m_State->GetActorChecked();
		}
		/**
		 * Returns whether the iterator has reached the end and no longer points
		 * to a suitable actor.
		 *
		 * @return true if iterator points to a suitable actor, false if it has reached the end
		 */
		FORCEINLINE explicit operator bool() const
		{
			return !m_State->m_ReachedEnd;
		}

		/**
		 * Clears the current Actor in the array (setting it to NULL).
		 */
		void ClearCurrent()
		{
			KR_CORE_ASSERT(!m_State->m_ReachedEnd, "");
			m_State->m_CurrentWorld->RemoveActor(m_State->m_CurrentActor, true);
		}

		/**
		 * Returns the number of actors considered thus far. Can be used in combination
		 * with GetProgressDenominator to gauge progress iterating over all actors.
		 *
		 * @return number of actors considered thus far.
		 */
		int32 GetProgressNumerator() const
		{
			return m_State->m_ConsideredCount;
		}

		/**
		 * Used to examine whether this level is valid for iteration or not
		 * This function should be redefined (thus hiding this one) in a derived class if it wants special level filtering.
		 *
		 * @param Level the level to check for iteration
		 * @return true if the level can be iterated, false otherwise
		 */
		FORCEINLINE bool CanIterateLevel(const ULevel* Level) const
		{
			/*
			if (EnumHasAnyFlags(Flags, EActorIteratorFlags::OnlyActiveLevels))
			{
				const bool bIsLevelVisibleOrAssociating = (Level->bIsVisible && !Level->bIsBeingRemoved) || Level->bIsAssociatingLevel || Level->bIsDisassociatingLevel;

				// Only allow iteration of Level if it's in the currently active level collection of the world, or is a static level.
				const FLevelCollection* const ActorLevelCollection = Level->GetCachedLevelCollection();
				const FLevelCollection* const ActiveLevelCollection = Level->OwningWorld ? Level->OwningWorld->GetActiveLevelCollection() : nullptr;

				// If the world's active level collection is null, we can't apply any meaningful filter,
				// so just allow iteration in this case.
				const bool bIsCurrentLevelCollectionTicking = !ActiveLevelCollection || (ActorLevelCollection == ActiveLevelCollection);

				const bool bIsLevelCollectionNullOrStatic = !ActorLevelCollection || ActorLevelCollection->GetType() == ELevelCollectionType::StaticLevels;
				const bool bShouldIterateLevelCollection = bIsCurrentLevelCollectionTicking || bIsLevelCollectionNullOrStatic;

				return bIsLevelVisibleOrAssociating && bShouldIterateLevelCollection;
			}*/

			return true;
		}

		/**
		 * Determines whether this is a valid actor or not.
		 * This function should be redefined (thus hiding this one) in a derived class if it wants special actor filtering.
		 *
		 * @param	Actor	Actor to check
		 * @return	true
		 */
		FORCEINLINE bool IsActorSuitable(const AActor* Actor) const
		{
			if (EnumHasAnyFlags(m_Flags, EActorIteratorFlags::SkipPendingKill) && !IsValid(Actor))
			{
				return false;
			}

			if (EnumHasAnyFlags(m_Flags, EActorIteratorFlags::OnlySelectedActors) && !Actor->IsSelected())
			{
				return false;
			}

			return true;
		}
	};
}
