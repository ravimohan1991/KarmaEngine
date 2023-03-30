#pragma once

#include "krpch.h"

#include "glm/glm.hpp"

namespace Karma
{
#define KR_SMALL_NUMBER			(1.e-8f)

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
		TRotator();
		TRotator(glm::vec3 EulerAngles);

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
		FTransform();
		FTransform(glm::vec3 rotation, glm::vec3 translation, glm::vec3 scale3D);

	public:
		const TRotator& GetRotation() const { return m_Rotation; }
		const glm::vec3& GetTranslation() const { return m_Translation; }
		const glm::vec3& GetScale3D() const { return m_Scale3D; }
		static const FTransform& Identity();

		FTransform GetRelativeTransform(const FTransform& RelativeToWhat) const;

		/**
		 * Return a transform that is the result of this multiplied by another transform.
		 * Order matters when composing transforms : C = A * B will yield a transform C that logically first applies A then B to any subsequent transformation.
		 *
		 * @param  Other other transform by which to multiply.
		 * @return new transform: this * Other
		 */
		inline FTransform operator*(const FTransform& Other) const;

		/**
		 * Create a new transform: OutTransform = A * B.
		 *
		 * Order matters when composing transforms : A * B will yield a transform that logically first applies A then B to any subsequent transformation.
		 *
		 * @param  OutTransform pointer to transform that will store the result of A * B.
		 * @param  A Transform A.
		 * @param  B Transform B.
		 */
		inline static void Multiply(FTransform* OutTransform, const FTransform* A, const FTransform* B);

		inline bool static AnyHasNegativeScale(const glm::vec3& InScale3D, const glm::vec3& InOtherScale3D)
		{
			return  (InScale3D.x < 0.f || InScale3D.y < 0.f || InScale3D.z < 0.f
				|| InOtherScale3D.x < 0.f || InOtherScale3D.y < 0.f || InOtherScale3D.z < 0.f);
		}

		/** 
		 * Mathematically if you have 0 scale, it should be infinite, 
		 * however, in practice if you have 0 scale, and relative transform doesn't make much sense 
		 * anymore because you should be instead of showing gigantic infinite mesh
		 * also returning BIG_NUMBER causes sequential NaN issues by multiplying 
		 * so we hardcode as 0
		 */
		inline static glm::vec3 GetSafeScaleReciprocal(const glm::vec3& InScale, long long Tolerance)
		{
			glm::vec3 SafeReciprocalScale;

			if (glm::abs(InScale.x) <= Tolerance)
			{
				SafeReciprocalScale.x = 0.f;
			}
			else
			{
				SafeReciprocalScale.x = 1 / InScale.x;
			}

			if (glm::abs(InScale.y) <= Tolerance)
			{
				SafeReciprocalScale.y = 0.f;
			}
			else
			{
				SafeReciprocalScale.y = 1 / InScale.y;
			}

			if (glm::abs(InScale.z) <= Tolerance)
			{
				SafeReciprocalScale.z = 0.f;
			}
			else
			{
				SafeReciprocalScale.z = 1 / InScale.z;
			}

			return SafeReciprocalScale;
		}


	private:
		/** Rotation of this transformation, need to work on quaternion <--> rotator conversions */
		TRotator m_Rotation;
		/** Translation of this transformation, as a vector */
		glm::vec3 m_Translation;
		/** 3D scale (always applied in local space) as a vector */
		glm::vec3 m_Scale3D;
	};
}