/**
 * @file Level.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the class ULevel.
 * @version 1.0
 * @date March 1, 2023
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "krpch.h"

#include "Core/Object.h"

namespace Karma
{
	class AActor;
	class UWorld;

	class AWorldSettings;

	/**
	 * @brief A Level is a collection of Actors (lights, volumes, mesh instances etc.).
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
		 * @brief The World that has this level in its Levels array.
		 * 
		 * This is not the same as GetOuter(), because GetOuter() for a streaming level is a vestigial world that is not used.
		 * It should not be accessed during BeginDestroy(), just like any other UObject references, since GC may occur in any order.
		 */
		UWorld* m_OwningWorld; // UE uses smart pointer

	public:
		/**
		 * @brief Initializes the Level appropriately
		 *
		 * @param InURL						The Karma's URL with which initialization may happen
		 *
		 * @todo Empty at the moment. Shall be filled in future with enough context.
		 * @see UWorld::InitializeNewWorld
		 */
		void Initialize(const FURL& InURL);

		/**
		 * @brief Set level's world settings
		 * @todo not completely functional yet
		 *
		 * @since Karma 1.0.0
		 */
		void SetWorldSettings(AWorldSettings* NewWorldSettings);

	public:
		/**
		 * @brief Override for UObject's GetWorld
		 * @note C++ final specifier is used 
		 *
		 * @return m_OwningWorld
		 * @since Karma 1.0.0
		 */
		virtual UWorld* GetWorld() const override final;

	private:
		/**
		 * Script accessible world properties for this level
		 */
		// TObjectPTR in UE
		AWorldSettings* m_WorldSettings;
	};
}
