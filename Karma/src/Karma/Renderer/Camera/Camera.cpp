#include "Camera.h"
#include "glm/gtc/matrix_transform.hpp"

namespace Karma
{
	Camera::Camera(glm::vec3& initialCameraPosition) : m_Position(initialCameraPosition)
	{
	}

	Camera::~Camera()
	{
	}

	void Camera::SetPosition(const glm::vec3& position)
	{
		m_Position = position;
		RecalculateViewProjectionMatrices();
	}

	void Camera::RecalculateViewProjectionMatrices()
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position)
			* glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0.0f, 0.0f, 1.0f));

		m_ViewMatrix = glm::inverse(transform);
		m_ViewProjectionMatirx = m_ProjectionMatrix * m_ViewMatrix;
	}
}