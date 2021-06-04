#include "PerspectiveCamera.h"
#include "Karma/Renderer/Renderer.h"
#include "glm/gtc/matrix_transform.hpp"

namespace Karma
{
	PerspectiveCamera::PerspectiveCamera(float fovRad, float aspectRatio, float nearPlane, float farPlane) :
		Camera(glm::vec3(0.0f, 0.0f, 1.5f))
	{
		m_CameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
		InitializePitchRoll();
		m_CameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
		m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_CameraFront, m_CameraUp);
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				KR_CORE_ASSERT(false, "RendererAPI::None is not supported");
				break;
			case RendererAPI::API::OpenGL:
				m_ProjectionMatrix = glm::perspective(glm::radians(fovRad), aspectRatio, nearPlane, farPlane);
				break;
			case RendererAPI::API::Vulkan:
				m_ProjectionMatrix = glm::perspective(glm::radians(fovRad), aspectRatio, nearPlane, farPlane);
				m_ProjectionMatrix[1][1] *= -1.0f;
				break;
		}
		m_ViewProjectionMatirx = m_ProjectionMatrix * m_ViewMatrix;
	}
}