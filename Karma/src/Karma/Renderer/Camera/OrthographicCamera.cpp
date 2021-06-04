#include "OrthographicCamera.h"
#include "Karma/Renderer/Renderer.h"
#include "glm/gtc/matrix_transform.hpp"

namespace Karma
{	
	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top) :
		Camera(glm::vec3(0.f))
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
				m_ProjectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
				break;
			case RendererAPI::API::Vulkan:
				m_ProjectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);// May want to adjust z range when supporting 3D
				m_ProjectionMatrix[1][1] *= -1.0f;
				break;
		}
		m_ViewProjectionMatirx = m_ProjectionMatrix * m_ViewMatrix;
	}
}