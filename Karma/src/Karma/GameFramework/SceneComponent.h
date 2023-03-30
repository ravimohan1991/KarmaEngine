#pragma once

#include "krpch.h"

#include "ActorComponent.h"
#include "glm/glm.hpp"

namespace Karma
{
	class FTransform;
	class USceneComponent;

	/** The space for the transform */
	enum ERelativeTransformSpace
	{
		/** World space transform. */
		RTS_World,
		/** Actor space transform. */
		RTS_Actor,
		/** Component space transform. */
		RTS_Component,
		/** Parent bone space transform */
		RTS_ParentBoneSpace,
	};

	/**
	 * A SceneComponent has a transform and supports attachment, but has no rendering or collision capabilities.
	 * Useful as a 'dummy' component in the hierarchy to offset others.
	 * @see [Scene Components](https://docs.unrealengine.com/latest/INT/Programming/UnrealArchitecture/Actors/Components/index.html#scenecomponents)
	 */
	class KARMA_API USceneComponent : public UActorComponent
	{
	public:
		USceneComponent();

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

		/** What we are currently attached to. If valid, RelativeLocation etc. are used relative to this object */
		std::shared_ptr<USceneComponent> m_AttachParent;

		/** Current transform of the component, relative to the world */
		FTransform m_ComponentToWorld;

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

		/**
		 * Gets the literal value of RelativeRotation.
		 * Note, this may be an absolute rotation if this is a root component (not attached to anything) or
		 * when GetAbsoluteRotation returns true.
		 *
		 * This exists so subclasses don't need to have direct access to the RelativeRotation property so it
		 * can be made private later.
		 */
		const glm::vec3& GetRelativeRotation() const
		{
			return m_RelativeRotation;
		}

		/**
		 * Gets the literal value of RelativeLocation.
		 * Note, this may be an absolute location if this is a root component (not attached to anything) or
		 * when IsUsingAbsoluteLocation returns true.
		 *
		 * This exists so subclasses don't need to have direct access to the RelativeLocation property so it
		 * can be made private later.
		 */
		const glm::vec3& GetRelativeLocation() const
		{
			return m_RelativeLocation;
		}

		/**
		 * Gets the literal value of RelativeScale3D.
		 * Note, this may be an absolute scale if this is a root component (not attached to anything) or
		 * when GetAbsoluteScale3D returns true.
		 *
		 * This exists so subclasses don't need to have direct access to the RelativeScale3D property so it
		 * can be made private later.
		 */
		const glm::vec3& GetRelativeScale3D() const
		{
			return m_RelativeScale3D;
		}

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

		/**
		 * Set the transform of the component in world space.
		 * @param NewTransform		New transform in world space for the component.
		 * 
		 * Rest are physics related
		 * @param SweepHitResult	Hit result from any impact if sweep is true.
		 * @param bSweep			Whether we sweep to the destination location, triggering overlaps along the way and stopping short of the target if blocked by something.
		 *							Only the root component is swept and checked for blocking collision, child components move without sweeping. If collision is off, this has no effect.
		 * @param bTeleport			Whether we teleport the physics state (if physics collision is enabled for this object).
		 *							If true, physics velocity for this object is unchanged (so ragdoll parts are not affected by change in location).
		 *							If false, physics velocity is updated based on the change in position (affecting ragdoll parts).
		 *							If CCD is on and not teleporting, this will affect objects along the entire sweep volume.
		 */
		void SetWorldTransform(const FTransform& NewTransform/*, bool bSweep = false, FHitResult* OutSweepHitResult = nullptr, ETeleportType Teleport = ETeleportType::None*/);

		/** Get the SceneComponent we are attached to. */
		std::shared_ptr<USceneComponent> GetAttachParent() const;

		/**
		 * Get world-space socket transform.
		 * @param					InSocketName Name of the socket or the bone to get the transform
		 * @return					Socket transform in world space if socket if found. Otherwise it will return component's transform in world space.
		 */
		virtual FTransform GetSocketTransform(const std::string& InSocketName, ERelativeTransformSpace TransformSpace = RTS_World) const;

		/** Get the current component-to-world transform for this component */
		inline const FTransform& GetComponentTransform() const
		{
			return m_ComponentToWorld;
		}
	};

	inline std::shared_ptr<USceneComponent> USceneComponent::GetAttachParent() const
	{
		return m_AttachParent;
	}
}