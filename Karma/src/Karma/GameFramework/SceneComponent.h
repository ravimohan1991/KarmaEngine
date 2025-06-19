/**
 * @file SceneComponent.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the class USceneComponent.
 * @version 1.0
 * @date March 1, 2023
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "krpch.h"

#include "ActorComponent.h"
#include "glm/glm.hpp"

#include "Ganit/Transform.h"

namespace Karma
{
	class FTransform;
	class USceneComponent;

	/** @brief The space for the transform */
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
	 * @brief A SceneComponent has a transform and supports attachment, but has no rendering or collision capabilities.
	 * Useful as a 'dummy' component in the hierarchy to offset others.
	 *
	 * @see [Scene Components](https://docs.unrealengine.com/latest/INT/Programming/UnrealArchitecture/Actors/Components/index.html#scenecomponents)
	 */
	class KARMA_API USceneComponent : public UActorComponent
	{
		DECLARE_KARMA_CLASS(USceneComponent, UActorComponent)

	public:
		/**
		 * @brief A constructor
		 *
		 * @since Karma 1.0.0
		 */
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

		/** If RelativeLocation should be considered relative to the world, rather than the parent */
		uint8_t m_bAbsoluteLocation : 1;

		/** If RelativeRotation should be considered relative to the world, rather than the parent */
		uint8_t m_bAbsoluteRotation : 1;

		/** If RelativeScale3D should be considered relative to the world, rather than the parent */
		uint8_t m_bAbsoluteScale : 1;

	public:
		/**
		 * Velocity of the component.
		 * @see GetComponentVelocity()
		*/
		glm::vec3 m_ComponentVelocity;

	public:
		/** What we are currently attached to. If valid, RelativeLocation etc. are used relative to this object */
		//std::shared_ptr<USceneComponent> m_AttachParent;

		/** Optional socket name on AttachParent that we are attached to. */
		std::string m_AttachSocketName;

		/** List of child SceneComponents that are attached to us. */
		std::vector<std::shared_ptr<USceneComponent>> m_AttachChildren;

		/**
		 * @brief Gets the literal value of RelativeRotation.
		 * @note This may be an absolute rotation if this is a root component (not attached to anything) or
		 * when GetAbsoluteRotation returns true.
		 *
		 * This exists so subclasses don't need to have direct access to the RelativeRotation property so it
		 * can be made private later.
		 *
		 * @since Karma 1.0.0
		 */
		const glm::vec3& GetRelativeRotation() const
		{
			return m_RelativeRotation;
		}

		/**
		 * @brief Gets the literal value of RelativeLocation.
		 * @note This may be an absolute location if this is a root component (not attached to anything) or
		 * when IsUsingAbsoluteLocation returns true.
		 *
		 * This exists so subclasses don't need to have direct access to the RelativeLocation property so it
		 * can be made private later.
		 *
		 * @since Karma 1.0.0
		 */
		const glm::vec3& GetRelativeLocation() const
		{
			return m_RelativeLocation;
		}

		/**
		 * @brief Gets the literal value of RelativeScale3D.
		 * @note this may be an absolute scale if this is a root component (not attached to anything) or
		 * when GetAbsoluteScale3D returns true.
		 *
		 * This exists so subclasses don't need to have direct access to the RelativeScale3D property so it
		 * can be made private later.
		 *
		 * @since Karma 1.0.0
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
		 *
		 * @since Karma 1.0.0
		 */
		void SetWorldLocation(glm::vec3 newLocation);

		/**
		 * Set the transform of the component in world space.
		 * @param NewTransform		New transform in world space for the component.
		 * 
		 * Rest are physics related. To be implemented in future.
		 * @param SweepHitResult	Hit result from any impact if sweep is true.
		 * @param bSweep			Whether we sweep to the destination location, triggering overlaps along the way and stopping short of the target if blocked by something.
		 *							Only the root component is swept and checked for blocking collision, child components move without sweeping. If collision is off, this has no effect.
		 * @param bTeleport			Whether we teleport the physics state (if physics collision is enabled for this object).
		 *							If true, physics velocity for this object is unchanged (so ragdoll parts are not affected by change in location).
		 *							If false, physics velocity is updated based on the change in position (affecting ragdoll parts).
		 *							If CCD is on and not teleporting, this will affect objects along the entire sweep volume.
		 * @since Karma 1.0.0
		 */
		void SetWorldTransform(const FTransform& NewTransform/*, bool bSweep = false, FHitResult* OutSweepHitResult = nullptr, ETeleportType Teleport = ETeleportType::None*/);

		/** 
		 * @brief Get the SceneComponent we are attached to.
		 *
		 * @since Karma 1.0.0
		 */
		std::shared_ptr<USceneComponent> GetAttachParent() const;

		/**
		 * @brief Get world-space socket transform.
		 *
		 * @param InSocketName 		Name of the socket or the bone to get the transform
		 * @return					Socket transform in world space if socket if found. Otherwise it will return component's transform in world space.
		 *
		 * @since Karma 1.0.0
		 */
		virtual FTransform GetSocketTransform(const std::string& InSocketName, ERelativeTransformSpace TransformSpace = RTS_World) const;

		/** 
		 * @brief Get the current component-to-world transform for this component
		 *
		 * @since Karma 1.0.0
		 */
		inline const FTransform& GetComponentTransform() const
		{
			return m_ComponentToWorld;
		}
		
		/**
		 * @brief Overridden BeginDestroy for USceneComponent
		 *
		 * @since Karma 1.0.0
		 */
		virtual void BeginDestroy() override;

		/**
		 * @brief Gets the literal value of bAbsoluteLocation.
		 *
		 * This exists so subclasses don't need to have direct access to the bAbsoluteLocation property so it
		 * can be made private later.
		 *
		 * @since Karma 1.0.0
		 */
		bool IsUsingAbsoluteLocation() const
		{
			return m_bAbsoluteLocation;
		}

		/**
		 * @brief Gets the literal value of bAbsoluteRotation.
		 *
		 * This exists so subclasses don't need to have direct access to the bAbsoluteRotation property so it
		 * can be made private later.
		 *
		 * @since Karma 1.0.0
		 */
		bool IsUsingAbsoluteRotation() const
		{
			return m_bAbsoluteRotation;
		}

		/**
		 * @brief Gets the literal value of bAbsoluteScale.
		 *
		 * This exists so subclasses don't need to have direct access to the bReplicates property so it
		 * can be made private later.
		 *
		 * @since Karma 1.0.0
		 */
		bool IsUsingAbsoluteScale() const
		{
			return m_bAbsoluteScale;
		}

		/**
		 * @brief Set the transform of the component relative to its parent
		 *
		 * @param NewTransform		New transform of the component relative to its parent.
		 * 
		 * To be activated once physics is enabled
		 * @param SweepHitResult	Hit result from any impact if sweep is true.
		 * @param bSweep			Whether we sweep to the destination (currently not supported for rotation).
		 * @param bTeleport			Whether we teleport the physics state (if physics collision is enabled for this object).
		 *							If true, physics velocity for this object is unchanged (so ragdoll parts are not affected by change in location).
		 *							If false, physics velocity is updated based on the change in position (affecting ragdoll parts).
		 *
		 * @since Karma 1.0.0
		 */
		void SetRelativeTransform(const FTransform& NewTransform/*, bool bSweep = false, FHitResult* OutSweepHitResult = nullptr, ETeleportType Teleport = ETeleportType::None*/);

		/**
		 * @brief Getter for the m_AttachSocektName
		 *
		 * @since Karma 1.0.0
		 */
		FORCEINLINE const std::string& GetAttachSocketName() const
		{
			return m_AttachSocketName;
		}
	};
}
