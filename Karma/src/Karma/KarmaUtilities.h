/**
 * @file KarmaUtilities.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains various classes and functions for various Karma's utilities
 * @version 1.0
 * @date June 2, 2022
 *
 * @copyright Karma Engine copyright(c) People of India
 */
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

	/**
	 * @brief The basic utilities class
	 */
	class KARMA_API KarmaUtilities
	{
	public:
		/**
		 * @brief Generates a sting of text contained within a file
		 *
		 * @param filePath						The relative path of the location of the file to be read
		 *
		 * @see Input::SetGamepadMapping()
		 * @since Karma 1.0.0
		 */
		static std::string ReadFileToSpitString(const std::string& filePath);

		/**
		 * @brief Extracts the file path from file source string (filepath + filename.extention)
		 *
		 * @param str						filepath + filename.extention string
		 * @since Karma 1.0.0
		 */
		static std::string GetFilePath(const std::string& str);

		/**
		 * @brief Gathers image pixel data, arranged left-to-right, top-to-bottom, for the supplied image file
		 *
		 * Few things to note:
		 * 1. Limitations:
		 * 		- no 12-bit-per-channel JPEG
		 * 		- no JPEGs with arithmetic coding
		 * 		- GIF always returns *comp (or *channels) = 4
		 * 2. The return value is an 'unsigned char *' which points
		 * 	to the pixel data, or NULL on an allocation failure or if the image is
		 * 	corrupt or invalid. The pixel data consists of *height scanlines of *width pixels,
		 * 	with each pixel consisting of N interleaved 8-bit components; the first
		 * 	pixel pointed to is top-left-most in the image. There is no padding between
		 * 	image scanlines or between pixels, regardless of format. The number of
		 * 	components N is 'desired_channels' if desired_channels is non-zero, or
		 * 	channels otherwise. If desired_channels is non-zero,
		 * 	channels has the number of components that _would_ have been
		 * 	output otherwise. E.g. if you set desired_channels to 4, you will always
		 * 	get RGBA output, but you can check *channels to see if it's trivially
		 * 	opaque because e.g. there were only 3 channels in the source image.
		 *
		 * @param fileName						The relative path to the file location
		 * @param width							outputs image width in pixels
		 * @param height						outputs image height in pixels
		 * @param channels						outputs number of image components in image file
		 * @param req_comp						(also desired_channels), if non-zero, are number of image components requested in result (rgba?)
		 *
		 * @since Karma 1.0.0
		 */
		static unsigned char* GetImagePixelData(char const* fileName, int* width, int* height, int* channels, int req_comp);
	};

	/**
	 * @brief Abstract base class for actor iteration. Implements all operators and relies on IsActorSuitable
	 * to be overridden by derived class.
	 *
	 * @note that when Playing In Editor (when implemented), this will find actors only in CurrentWorld.
	 */
	class FActorIteratorState
	{
	public:
		/** Current world we are iterating upon*/
		const UWorld*						m_CurrentWorld;

		/** Results from the GetObjectsOfClass query*/
		KarmaVector<UObject*>				m_ObjectArray;

		/** Index of the current element in the object array*/
		int32_t								m_Index;

		/** Whether we already reached the end*/
		bool								m_ReachedEnd;

		/** Number of actors that have been considered thus far*/
		int32_t								m_ConsideredCount;

		/** Current actor pointed to by actor iterator*/
		AActor*								m_CurrentActor;

		/** Contains any actors spawned during iteration*/
		KarmaVector<AActor*>				m_SpawnedActorArray;

		/** The class type we are iterating, kept for filtering*/
		UClass*								m_DesiredClass;

		/** Handle to the registered OnActorSpawned delegate*/
		//FDelegateHandle ActorSpawnedDelegateHandle;

		/**
		 * @brief Default constructor
		 *
		 * @since Karma 1.0.0
		 */
		FActorIteratorState() {};

		/**
		 * @brief Default constructor, initializes everything relevant
		 *
		 * @param InWorld						The UWorld we would want to do iterations within
		 * @param InClass						The class type we are iterating, kept for filtering
		 *
		 * @since Karma 1.0.0
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

		/**
		 * @brief A destructor
		 *
		 * @since Karma 1.0.0
		 */
		~FActorIteratorState()
		{
			//m_CurrentWorld->RemoveOnActorSpawnedHandler(ActorSpawnedDelegateHandle);
		}


		/**
		 * @brief Returns the current suitable actor pointed at by the Iterator
		 *
		 * @return Current suitable actor
		 * @since Karma 1.0.0
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

	/** 
	 * @brief Iteration flags, specifies which types of levels and actors should be iterated
	 */
	enum EActorIteratorFlags
	{
		/**
		 * @brief No flags, iterate all actors
		 */
		AllActors			= 0x00000000,
		/**
		 * @brief Skip pending kill actors
		 */
		SkipPendingKill		= 0x00000001,
		/**
		 * @brief Only iterate actors that are selected
		 */
		OnlySelectedActors	= 0x00000002,
		/**
		 * @brief Only iterate active levels
		 */
		OnlyActiveLevels	= 0x00000004,
	};

	/** 
	 * @brief Type enum, used to represent the special End iterator
	 */
	enum class EActorIteratorType
	{
		End
	};

	/**
	 * @brief Template class used to filter actors by certain characteristics
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
		 * @brief A constructor
		 *
		 * @param EActorIteratorType						Specify the iterator flag from EActorIteratorFlags
		 *
		 * @note Hide the constructors as construction on this class should only be done by subclasses
		 * @since Karma 1.0.0
		 */
		TActorIteratorBase(EActorIteratorType)
			: m_Flags(EActorIteratorFlags::AllActors)
		{
		}

		/**
		 * @brief Constructor for setting m_State (FActorIteratorState) with the supplied InWorld and InClass
		 *
		 * @param InWorld						The UWorld we would like to do the iteration within
		 * @param InClass						The class type we are iterating, kept for filtering
		 *
		 * @param InFlags						Specify the iterator flag from EActorIteratorFlags
		 *
		 * @since Karma 1.0.0
		 */
		TActorIteratorBase(const UWorld* InWorld, TSubclassOf<AActor> InClass, const EActorIteratorFlags InFlags)
			: m_Flags(InFlags)
		{
			m_State.reset(new FActorIteratorState(InWorld, InClass));
		}

	public:
		/**
		 * @brief Iterates to next suitable actor.
		 *
		 * The pseudo code is chalked out in the wiki page https://github.com/ravimohan1991/KarmaEngine/wiki/Karma's-Actor-Iteration-Process
		 *
		 * @since Karma 1.0.0
		 */
		void operator++()
		{
			// Use local version to avoid LHSs as compiler is not required to write out member variables to memory.
			AActor* localCurrentActor = nullptr;
			int32_t             localIndex = m_State->m_Index;
			KarmaVector<UObject*>& localObjectArray = m_State->m_ObjectArray;// note that localObjectArray is a vector
			KarmaVector<AActor*>& localSpawnedActorArray = m_State->m_SpawnedActorArray; // Contains any actors spawned during iteration
			const UWorld* localCurrentWorld = m_State->m_CurrentWorld;

			int32_t lObjectArrayNum = 				int32_t(localObjectArray.Num());
			int32_t lSAArrayNum = 					int32_t(localSpawnedActorArray.Num());

			while (++localIndex < (lObjectArrayNum + lSAArrayNum))
			{
				if (localIndex < lObjectArrayNum)
				{
					localCurrentActor = static_cast<AActor*>(localObjectArray.GetElements()[localIndex]);
				}
				else
				{
					localCurrentActor = localSpawnedActorArray.GetElements()[localIndex - lObjectArrayNum];
				}
				m_State->m_ConsideredCount++;// Number of actors that have been considered thus far

				ULevel* actorLevel = localCurrentActor ? localCurrentActor->GetLevel() : nullptr;

				if (actorLevel
					&& static_cast<const Derived*>(this)->IsActorSuitable(localCurrentActor)
					&& static_cast<const Derived*>(this)->CanIterateLevel(actorLevel) && actorLevel->GetWorld() == localCurrentWorld)
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
		 * @brief Returns the current suitable actor pointed at by the Iterator
		 *
		 * @return	Current suitable actor
		 * @since Karma 1.0.0
		 */
		FORCEINLINE AActor* operator*() const
		{
			return m_State->GetActorChecked();
		}

		/**
		 * @brief Returns the current suitable actor pointed at by the Iterator
		 *
		 * @return	Current suitable actor
		 * @since Karma 1.0.0
		 */
		FORCEINLINE AActor* operator->() const
		{
			return m_State->GetActorChecked();
		}

		/**
		 * @brief Returns whether the iterator has reached the end and no longer points
		 * to a suitable actor.
		 *
		 * @return true if iterator points to a suitable actor, false if it has reached the end
		 * @since Karma 1.0.0
		 */
		FORCEINLINE explicit operator bool() const
		{
			return !m_State->m_ReachedEnd;
		}

		/**
		 * @brief Clears the current Actor in the array (setting it to NULL).
		 *
		 * @since Karma 1.0.0
		 */
		void ClearCurrent()
		{
			KR_CORE_ASSERT(!m_State->m_ReachedEnd, "");
			m_State->m_CurrentWorld->RemoveActor(m_State->m_CurrentActor, true);
		}

		/**
		 * @brief Returns the number of actors considered thus far. Can be used in combination
		 * with GetProgressDenominator to gauge progress iterating over all actors.
		 *
		 * @return number of actors considered thus far.
		 * @since Karma 1.0.0
		 */
		int32 GetProgressNumerator() const
		{
			return m_State->m_ConsideredCount;
		}

		/**
		 * @brief Used to examine whether this level is valid for iteration or not.
		 * This function should be redefined (thus hiding this one) in a derived class if it wants special level filtering.
		 *
		 * @param Level						the level to check for iteration
		 *
		 * @return true if the level can be iterated, false otherwise
		 * @since Karma 1.0.0
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
		 * @brief Determines whether this is a valid actor or not.
		 * This function should be redefined (thus hiding this one) in a derived class if it wants special actor filtering.
		 *
		 * @param	Actor	Actor to check
		 *
		 * @return	true
		 * @since Karma 1.0.0
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
