#pragma once

#include "glm/glm.hpp"
#include "Karma/Core.h"

namespace Karma
{
	class KARMA_API Camera
	{
	public:
        Camera(const glm::vec3& initialCameraPosition);
		virtual ~Camera();

		const glm::vec3& GetPosition() const { return m_Position; }
		float GetRotation() const { return m_Rotation; }

		void LeftMouseButtonPressed();
		void LeftMouseButtonReleased();
		void ComputeCameraFront();
		
		void SetPosition(const glm::vec3& position);
		void SetRotation(float rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }
		inline void SetCameraFront(const glm::vec3& front) { m_CameraFront = front; }
		inline void SetCameraUp(const glm::vec3& up) { m_CameraUp = up; }

		void MoveForward(float amount);
		void MoveSideways(float amount);
		void MoveUp(float amount);
		void RotateAboutYAxis(float amount);
		void RotateAboutXAxis(float amount);

		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		const glm::mat4& GetViewMatirx() const { return m_ViewMatrix; }

	private:
		void RecalculateViewMatrix();

	protected:
		void InitializePitchRoll();
	
	protected:
		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;

		glm::vec3 m_Position;
		glm::vec3 m_CameraFront;
		glm::vec3 m_CameraUp;
		float m_Rotation = 0.0f;

		bool m_IsLeftMouseButtonPressed = false;
		float m_LastMouseX, m_LastMouseY;

		const float m_Sensitivity = 0.1f;
		float m_Roll = 0.0f;
		float m_Pitch = 0.0f;
	};
}
