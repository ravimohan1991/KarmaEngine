#include "Transform.h"

namespace Karma
{
	FTransform FTransform::m_Identity = FTransform();

	TRotator::TRotator()
	{
		m_Yaw = 0.0f;
		m_Pitch = 0.0f;
		m_Roll = 0.0f;
	}

	TRotator::TRotator(glm::vec3 eulerAngles)
	{
		m_Yaw = eulerAngles.y;
		m_Pitch = eulerAngles.z;
		m_Roll = eulerAngles.x;
	}

	TRotator TRotator::Inverse() const
	{
		TRotator returnRotation = TRotator(glm::vec3(-m_Yaw, -m_Pitch, -m_Roll));
		return returnRotation;
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

	FTransform FTransform::Identity()
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
			Result.SetScale3D(m_Scale3D * SafeRecipScale3D);

			/* What is this?
			if (RelativeToWhat.GetRotation().IsNormalized() == false)
			{
				return Identity();
			}
			*/

			TRotator Inverse = RelativeToWhat.GetRotation().Inverse();
			Result.SetRotation(Inverse * m_Rotation);

			glm::vec3 relativeRotatedTranslation = Inverse * (m_Translation - RelativeToWhat.GetTranslation());

			Result.SetTranslation(glm::vec3(relativeRotatedTranslation.x * SafeRecipScale3D.x,
											relativeRotatedTranslation.y * SafeRecipScale3D.y,
											relativeRotatedTranslation.z * SafeRecipScale3D.z));
		}

		return Result;
	}

	FTransform FTransform::operator*(const FTransform& Other) const
	{
		FTransform someTransform;
		Multiply(&someTransform, this, &Other);

		return someTransform;
	}

	/** Returns Multiplied Transform of 2 FTransforms **/
	inline void FTransform::Multiply(FTransform* OutTransform, const FTransform* A, const FTransform* B)
	{
		//A->DiagnosticCheckNaN_All();
		//B->DiagnosticCheckNaN_All();

		//checkSlow(A->IsRotationNormalized());
		//checkSlow(B->IsRotationNormalized());

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
		//checkSlow(VectorGetComponent(A->Scale3D, 3) == 0.f);
		//checkSlow(VectorGetComponent(B->Scale3D, 3) == 0.f);

		//if (Private_AnyHasNegativeScale(A->Scale3D, B->Scale3D))
		//{
			// @note, if you have 0 scale with negative, you're going to lose rotation as it can't convert back to quat
		//	MultiplyUsingMatrixWithScale(OutTransform, A, B);
		//}
		//else
		//{
			const TRotator RotationA = A->GetRotation();
			const TRotator RotationB = B->GetRotation();
			const glm::vec3 TranslateA = A->GetTranslation();
			const glm::vec3 TranslateB = B->GetTranslation();
			const glm::vec3 ScaleA = A->GetScale3D();
			const glm::vec3 ScaleB = B->GetScale3D();

			// RotationResult = B.Rotation * A.Rotation
			OutTransform->SetRotation(TRotator(glm::vec3(RotationA.m_Pitch + RotationB.m_Pitch, RotationA.m_Yaw + RotationB.m_Pitch, RotationA.m_Roll + RotationB.m_Roll)));

			// TranslateResult = B.Rotate(B.Scale * A.Translation) + B.Translate
			const glm::vec3 ScaledTransA(TranslateA.x * ScaleB.x, TranslateA.y * ScaleB.y, TranslateA.z * ScaleB.z);// = VectorMultiply(TranslateA, ScaleB);
			const glm::vec3 RotatedTranslate(RotationB * ScaledTransA);// = VectorQuaternionRotateVector(QuatB, ScaledTransA);
			OutTransform->SetTranslation(RotatedTranslate + TranslateB);// = VectorAdd(RotatedTranslate, TranslateB);

			// ScaleResult = Scale.B * Scale.A
			OutTransform->SetScale3D(glm::vec3(ScaleA.x * ScaleB.x, ScaleA.y * ScaleB.y, ScaleA.z * ScaleB.z));// = VectorMultiply(ScaleA, ScaleB);
		//}
	}
}
