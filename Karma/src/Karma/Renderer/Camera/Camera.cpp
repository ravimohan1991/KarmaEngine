#include "Camera.h"
#include "glm/gtc/matrix_transform.hpp"
#include "Karma/Input.h"

namespace Karma
{	
	Camera::Camera(glm::vec3& initialCameraPosition) : m_Position(initialCameraPosition), m_LastMouseX(0.0f), m_LastMouseY(0.0f)
	{
	}

	Camera::~Camera()
	{
	}

	void Camera::SetPosition(const glm::vec3& position)
	{
		m_Position = position;
		RecalculateViewMatrix();
	}

	void Camera::RecalculateViewMatrix()
	{
		m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_CameraFront, m_CameraUp);
	}

	void Camera::MoveForward(float amount)
	{
		m_Position += amount * m_CameraFront;
		RecalculateViewMatrix();
	}

	void Camera::MoveSideways(float amount)
	{
		m_Position += amount * glm::normalize(glm::cross(m_CameraFront, m_CameraUp));
		RecalculateViewMatrix();
	}

	void Camera::MoveUp(float amount)
	{
		m_Position += amount * m_CameraUp;
		RecalculateViewMatrix();
	}

	void Camera::LeftMouseButtonPressed()
	{
		std::pair mousePos = Input::GetMousePosition();
		if (!m_IsLeftMouseButtonPressed)
		{
			m_LastMouseX = mousePos.first;
			m_LastMouseY = mousePos.second;
			m_IsLeftMouseButtonPressed = true;
			return;
		}
		float xOffset = (mousePos.first - m_LastMouseX) * m_Sensitivity;
		float yOffset = (m_LastMouseY - mousePos.second) * m_Sensitivity;
		if (xOffset != 0.0f || yOffset != 0.0f)
		{
			m_LastMouseX = mousePos.first;
			m_LastMouseY = mousePos.second;

			m_Pitch += glm::radians(xOffset);
			m_Roll += glm::radians(yOffset);

			ComputeCameraFront();
			RecalculateViewMatrix();
		}
	}

	void Camera::LeftMouseButtonReleased()
	{
		m_IsLeftMouseButtonPressed = false;
	}

	void Camera::ComputeCameraFront()
	{
		glm::vec3 direction;
		direction.x = cos(m_Pitch) * cos(m_Roll);
		direction.y = sin(m_Roll);
		direction.z = sin(m_Pitch) * cos(m_Roll);
		m_CameraFront = glm::normalize(direction);
	}

	void Camera::InitializePitchRoll()
	{
		m_Pitch = glm::atan(m_CameraFront.z / m_CameraFront.x);
		m_Roll = glm::atan(m_CameraFront.y / glm::sqrt(glm::pow(m_CameraFront.x, 2) + glm::pow(m_CameraFront.z, 2)));
	}
}