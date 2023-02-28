#pragma once

#include "krpch.h"

#include "Core/Object.h"

namespace Karma
{
	class AActor;

	/**
	 * A Level is a collection of Actors (lights, volumes, mesh instances etc.).
	 * Multiple Levels can be loaded and unloaded into the World to create a streaming experience.
	 *
	 * @see https://docs.unrealengine.com/latest/INT/Engine/Levels
	 * @see UActor
	 */
	class KARMA_API ULevel : public UObject
	{
	public:

		/** URL associated with this level. */
		std::string					URL;

		/** Array of all actors in this level, used by FActorIteratorBase and derived classes */
		std::vector<AActor*> Actors;

		/** Array of actors to be exposed to GC in this level. All other actors will be referenced through ULevelActorContainer */
		std::vector<AActor*> ActorsForGC;
	};
}