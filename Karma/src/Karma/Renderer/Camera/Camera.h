#pragma once

#include "glm/glm.hpp"
#include "Karma/Core.h"

namespace Karma
{
	class KARMA_API Camera
	{
	public:
		Camera();
		virtual ~Camera();

		const glm::vec3& GetPosition() const { return m_Position; }
		float GetRotation() const { return m_Rotation; }

		void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateViewProjectionMatrices(); }
		void SetRotation(float rotation) { m_Rotation = rotation; RecalculateViewProjectionMatrices(); }

		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		const glm::mat4& GetViewMatirx() const { return m_ViewMatrix; }
		const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatirx; }

	private:
		void RecalculateViewProjectionMatrices();
	
	protected:
		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ViewProjectionMatirx;

		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		float m_Rotation = 0.0f;
	};
}