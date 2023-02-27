#pragma once

#include "krpch.h"

#include "ActorComponent.h"
#include "glm/glm.hpp"

namespace Karma
{
	/**
	 * A SceneComponent has a transform and supports attachment, but has no rendering or collision capabilities.
	 * Useful as a 'dummy' component in the hierarchy to offset others.
	 * @see [Scene Components](https://docs.unrealengine.com/latest/INT/Programming/UnrealArchitecture/Actors/Components/index.html#scenecomponents)
	 */
	class KARMA_API USceneComponent : public UActorComponent
	{
	public:
		/** Returns the transform of the component relative to its parent */
		//FTransform GetRelativeTransform() const;

	private:
		/** Location of the component relative to its parent */
		glm::vec3 m_RelativeLocation;

		/** Rotation of the component relative to its parent */
		glm::vec3 m_RelativeRotation;

		/**
		*	Non-uniform scaling of the component relative to its parent.
		*	Note that scaling is always applied in local space (no shearing etc)
		*/
		glm::vec3 m_RelativeScale3D;

	public:
		/**
		* Velocity of the component.
		* @see GetComponentVelocity()
		*/
		glm::vec3 m_ComponentVelocity;

	public:
		/** What we are currently attached to. If valid, RelativeLocation etc. are used relative to this object */
		std::shared_ptr<USceneComponent> m_AttachParent;

		/** Optional socket name on AttachParent that we are attached to. */
		std::string m_AttachSocketName;

		/** List of child SceneComponents that are attached to us. */
		std::vector<std::shared_ptr<USceneComponent>> m_AttachChildren;

	public:
		/**
		 * Put this component at the specified location in world space. Updates relative location to achieve the final world location.
		 * @param NewLocation		New location in world space for the component.
		 * 
		 * These params shall be incorporated later in dev cycle.
		 * @param SweepHitResult	Hit result from any impact if sweep is true.
		 * @param bSweep			Whether we sweep to the destination location, triggering overlaps along the way and stopping short of the target if blocked by something.
		 *							Only the root component is swept and checked for blocking collision, child components move without sweeping. If collision is off, this has no effect.
		 * @param bTeleport			Whether we teleport the physics state (if physics collision is enabled for this object).
		 *							If true, physics velocity for this object is unchanged (so ragdoll parts are not affected by change in location).
		 *							If false, physics velocity is updated based on the change in position (affecting ragdoll parts).
		 *							If CCD is on and not teleporting, this will affect objects along the entire sweep volume.
		 */
		void SetWorldLocation(glm::vec3 newLocation);
	};
}