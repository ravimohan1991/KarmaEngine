#include "Transform.h"

namespace Karma
{
	TRotator::TRotator()
	{
		m_Yaw = glm::vec1(0.0f);
		m_Pitch = glm::vec1(0.0f);
		m_Roll = glm::vec1(0.0f);
	}

	FTransform::FTransform()
	{
		m_Translation = glm::vec3(0.0f, 0.0f, 0.0f);
		m_Rotation = TRotator();
		m_Scale3D = glm::vec3(0.0f, 0.0f, 0.0f);
	}

	const FTransform&  FTransform::Identity()
	{
		return FTransform();
	}
}