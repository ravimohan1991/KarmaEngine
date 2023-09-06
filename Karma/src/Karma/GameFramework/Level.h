#pragma once

#include "krpch.h"

#include "Core/Object.h"

namespace Karma
{
	class AActor;
	class UWorld;

	class AWorldSettings;

	/**
	 * A Level is a collection of Actors (lights, volumes, mesh instances etc.).
	 * Multiple Levels can be loaded and unloaded into the World to create a streaming experience.
	 *
	 * @see https://docs.unrealengine.com/latest/INT/Engine/Levels
	 */
	class KARMA_API ULevel : public UObject
	{
		DECLARE_KARMA_CLASS(ULevel, UObject)

	public:
		ULevel();

	public:

		/** URL associated with this level. */
		FURL					m_URL;

		/** Array of all actors in this level, used by FActorIteratorBase and derived classes */
		KarmaVector<AActor*> m_Actors;

		/** Array of actors to be exposed to GC in this level. All other actors will be referenced through ULevelActorContainer */
		std::vector<AActor*> m_ActorsForGC;

		/**
		 * The World that has this level in its Levels array.
		 * This is not the same as GetOuter(), because GetOuter() for a streaming level is a vestigial world that is not used.
		 * It should not be accessed during BeginDestroy(), just like any other UObject references, since GC may occur in any order.
		 */
		UWorld* m_OwningWorld; // UE uses smart pointer

	public:
		void Initialize(const FURL& InURL);

		/**
		 * Set level's world settings
		 * Caution: not completely functional yet
		 */
		void SetWorldSettings(AWorldSettings* NewWorldSettings);

	public:
		virtual UWorld* GetWorld() const override final;

	private:
		/**
		 * Script accessible world properties for this level
		 */
		// TObjectPTR in UE
		AWorldSettings* m_WorldSettings;
	};
}
