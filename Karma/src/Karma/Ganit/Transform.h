#pragma once

#include "krpch.h"

#include "glm/glm.hpp"

namespace Karma
{
	/**
	 * Floating point quaternion that can represent a rotation about an axis in 3-D space.
	 * The X, Y, Z, W components also double as the Axis/Angle format.
	 *
	 * Order matters when composing quaternions: C = A * B will yield a quaternion C that logically
	 * first applies B then A to any subsequent transformation (right first, then left).
	 * Note that this is the opposite order of FTransform multiplication.
	 *
	 * Example: LocalToWorld = (LocalToWorld * DeltaRotation) will change rotation in local space by DeltaRotation.
	 * Example: LocalToWorld = (DeltaRotation * LocalToWorld) will change rotation in world space by DeltaRotation.
	 */
	struct KARMA_API TQuaternion
	{
	public:
		/** The quaternion's X-component. */
		glm::vec1 X;

		/** The quaternion's Y-component. */
		glm::vec1 Y;

		/** The quaternion's Z-component. */
		glm::vec1 Z;

		/** The quaternion's W-component. */
		glm::vec1 W;
	};

	/**
	 * Implements a container for rotation information.
	 *
	 * All rotation values are stored in degrees.
	 *
	 * The angles are interpreted as intrinsic rotations applied in the order Yaw, then Pitch, then Roll. I.e., an object would be rotated
	 * first by the specified yaw around its up axis (with positive angles interpreted as clockwise when viewed from above, along -Z),
	 * then pitched around its (new) right axis (with positive angles interpreted as 'nose up', i.e. clockwise when viewed along +Y),
	 * and then finally rolled around its (final) forward axis (with positive angles interpreted as clockwise rotations when viewed along +X).
	 *
	 * Note that these conventions differ from quaternion axis/angle. UE Quat always considers a positive angle to be a left-handed rotation,
	 * whereas Rotator treats yaw as left-handed but pitch and roll as right-handed.
	 *
	 */
	struct KARMA_API TRotator
	{
		/** Rotation around the right axis (around Y axis), Looking up and down (0=Straight Ahead, +Up, -Down) */
		glm::vec1 m_Pitch;

		/** Rotation around the up axis (around Z axis), Turning around (0=Forward, +Right, -Left)*/
		glm::vec1 m_Yaw;

		/** Rotation around the forward axis (around X axis), Tilting your head, (0=Straight, +Clockwise, -CCW) */
		glm::vec1 m_Roll;
	};


	/**
	 * Transform composed of Scale, Rotation (as a quaternion), and Translation.
	 *
	 * Transforms can be used to convert from one space to another, for example by transforming
	 * positions and directions from local space to world space.
	 *
	 * Transformation of position vectors is applied in the order:  Scale -> Rotate -> Translate.
	 * Transformation of direction vectors is applied in the order: Scale -> Rotate.
	 *
	 * Order matters when composing transforms: C = A * B will yield a transform C that logically
	 * first applies A then B to any subsequent transformation. Note that this is the opposite order of quaternion (TQuat<T>) multiplication.
	 *
	 * Example: LocalToWorld = (DeltaRotation * LocalToWorld) will change rotation in local space by DeltaRotation.
	 * Example: LocalToWorld = (LocalToWorld * DeltaRotation) will change rotation in world space by DeltaRotation.
	 */
	class KARMA_API FTransform
	{
	public:
		const TRotator& GetRotation() const { return m_Rotation; }
		const glm::vec3& GetTranslation() const { return m_Translation; }
		const glm::vec3& GetScale3D() const { return m_Scale3D; }

	private:
		/** Rotation of this transformation, need to work on quaternion <--> rotator conversions */
		TRotator m_Rotation;
		/** Translation of this transformation, as a vector */
		glm::vec3 m_Translation;
		/** 3D scale (always applied in local space) as a vector */
		glm::vec3 m_Scale3D;
	};
}