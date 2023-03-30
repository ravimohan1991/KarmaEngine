#include "Transform.h"

namespace Karma
{
	TRotator::TRotator()
	{
		m_Yaw = glm::vec1(0.0f);
		m_Pitch = glm::vec1(0.0f);
		m_Roll = glm::vec1(0.0f);
	}

	TRotator::TRotator(glm::vec3 eulerAngles)
	{
		m_Yaw = glm::vec1(eulerAngles.y);
		m_Pitch = glm::vec1(eulerAngles.z);
		m_Roll = glm::vec1(eulerAngles.x);
	}

	FTransform::FTransform()
	{
		m_Translation = glm::vec3(0.0f, 0.0f, 0.0f);
		m_Rotation = TRotator();
		m_Scale3D = glm::vec3(1.0f, 1.0f, 1.0f);
	}

	FTransform::FTransform(glm::vec3 rotation, glm::vec3 translation, glm::vec3 scale3D) :
		m_Rotation(TRotator(rotation)),
		m_Translation(translation),
		m_Scale3D(scale3D)
	{
	}

	const FTransform&  FTransform::Identity()
	{
		return FTransform();
	}

	FTransform FTransform::GetRelativeTransform(const FTransform& RelativeToWhat) const
	{
		// A * B(-1) = VQS(B)(-1) (VQS (A))
		// 
		// Scale = S(A)/S(B)
		// Rotation = Q(B)(-1) * Q(A)
		// Translation = 1/S(B) *[Q(B)(-1)*(T(A)-T(B))*Q(B)]
		// where A = this, B = RelativeToWhat
		FTransform Result;

		if (AnyHasNegativeScale(m_Scale3D, RelativeToWhat.GetScale3D()))
		{
			// @note, if you have 0 scale with negative, you're going to lose rotation as it can't convert back to quat
			KR_CORE_ASSERT(false, "Not supporting negative scaling for now");
			// GetRelativeTransformUsingMatrixWithScale(&Result, this, &RelativeToWhat);
		}
		else
		{
			glm::vec3 SafeRecipScale3D = GetSafeScaleReciprocal(RelativeToWhat.m_Scale3D, KR_SMALL_NUMBER);
			Result.m_Scale3D = m_Scale3D * SafeRecipScale3D;

			if (RelativeToWhat.m_Rotation.IsNormalized() == false)
			{
				return Identity();
			}

			TQuat<T> Inverse = Other.Rotation.Inverse();
			Result.Rotation = Inverse * Rotation;

			Result.Translation = (Inverse * (Translation - Other.Translation)) * (SafeRecipScale3D);
		}

		return Result;
	}

	/** Returns Multiplied Transform of 2 FTransforms **/
	inline void FTransform::Multiply(FTransform* OutTransform, const FTransform* A, const FTransform* B)
	{
		A->DiagnosticCheckNaN_All();
		B->DiagnosticCheckNaN_All();

		checkSlow(A->IsRotationNormalized());
		checkSlow(B->IsRotationNormalized());

		//	When Q = quaternion, S = single scalar scale, and T = translation
		//	QST(A) = Q(A), S(A), T(A), and QST(B) = Q(B), S(B), T(B)

		//	QST (AxB) 

		// QST(A) = Q(A)*S(A)*P*-Q(A) + T(A)
		// QST(AxB) = Q(B)*S(B)*QST(A)*-Q(B) + T(B)
		// QST(AxB) = Q(B)*S(B)*[Q(A)*S(A)*P*-Q(A) + T(A)]*-Q(B) + T(B)
		// QST(AxB) = Q(B)*S(B)*Q(A)*S(A)*P*-Q(A)*-Q(B) + Q(B)*S(B)*T(A)*-Q(B) + T(B)
		// QST(AxB) = [Q(B)*Q(A)]*[S(B)*S(A)]*P*-[Q(B)*Q(A)] + Q(B)*S(B)*T(A)*-Q(B) + T(B)

		//	Q(AxB) = Q(B)*Q(A)
		//	S(AxB) = S(A)*S(B)
		//	T(AxB) = Q(B)*S(B)*T(A)*-Q(B) + T(B)
		checkSlow(VectorGetComponent(A->Scale3D, 3) == 0.f);
		checkSlow(VectorGetComponent(B->Scale3D, 3) == 0.f);

		if (Private_AnyHasNegativeScale(A->Scale3D, B->Scale3D))
		{
			// @note, if you have 0 scale with negative, you're going to lose rotation as it can't convert back to quat
			MultiplyUsingMatrixWithScale(OutTransform, A, B);
		}
		else
		{
			const TransformVectorRegister QuatA = A->Rotation;
			const TransformVectorRegister QuatB = B->Rotation;
			const TransformVectorRegister TranslateA = A->Translation;
			const TransformVectorRegister TranslateB = B->Translation;
			const TransformVectorRegister ScaleA = A->Scale3D;
			const TransformVectorRegister ScaleB = B->Scale3D;

			// RotationResult = B.Rotation * A.Rotation
			OutTransform->Rotation = VectorQuaternionMultiply2(QuatB, QuatA);

			// TranslateResult = B.Rotate(B.Scale * A.Translation) + B.Translate
			const TransformVectorRegister ScaledTransA = VectorMultiply(TranslateA, ScaleB);
			const TransformVectorRegister RotatedTranslate = VectorQuaternionRotateVector(QuatB, ScaledTransA);
			OutTransform->Translation = VectorAdd(RotatedTranslate, TranslateB);

			// ScaleResult = Scale.B * Scale.A
			OutTransform->Scale3D = VectorMultiply(ScaleA, ScaleB);
		}
	}
}