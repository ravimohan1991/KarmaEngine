#pragma once

#include "krpch.h"

#include "SceneComponent.h"
#include "SubClassOf.h"

namespace Karma
{
	class AActor;

	/** A component that spawns an Actor when registered, and destroys it when unregistered.*/
	class KARMA_API UChildActorComponent : public USceneComponent
	{
		DECLARE_KARMA_CLASS(UChildActorComponent, USceneComponent)

	private:
		/** The class of Actor to spawn */
		TSubclassOf<AActor>	m_ChildActorClass;

		/** The actor that we spawned and own */
		// ObjectPtr
		AActor* m_ChildActor;

		/** Property to point to the template child actor for details panel purposes */
		// ObjectPtr
		AActor* m_ChildActorTemplate;

		/** We try to keep the child actor's name as best we can, so we store it off here when destroying */
		std::string m_ChildActorName;

		/** Detect when the parent actor is renamed, in which case we can't preseve the child actor's name */
		UObject* m_ActorOuter;

		/** Cached copy of the instance data when the ChildActor is destroyed to be available when needed */
		//mutable FChildActorComponentInstanceData* CachedInstanceData;

		/** Flag indicating that when the component is registered that the child actor should be recreated */
		uint8_t m_bNeedsRecreate : 1;

	public:
		virtual void BeginDestroy() override;
	};
}