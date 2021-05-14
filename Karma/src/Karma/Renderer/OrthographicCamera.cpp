#include "OrthographicCamera.h"
#include "Karma/Renderer/Renderer.h"
#include "glm/gtc/matrix_transform.hpp"

namespace Karma
{
	OrthographicCamera::OrthographicCamera()
	{
	}
	
	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top)
		: m_ViewMatrix(1.0f)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				KR_CORE_ASSERT(false, "RendererAPI::None is not supported");
				break;
			case RendererAPI::API::OpenGL:
				m_ProjectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
				break;
			case RendererAPI::API::Vulkan:
				m_ProjectionMatrix = glm::ortho(left, right, top, bottom, -1.0f, 1.0f);// May want to adjust z range when supporting 3D
				break;
		}
		m_ViewProjectionMatirx = m_ProjectionMatrix * m_ViewMatrix;
	}

	void OrthographicCamera::RecalculateViewMatrix()
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position) 
			* glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0.0f, 0.0f, 1.0f));

		m_ViewMatrix = glm::inverse(transform);
		m_ViewProjectionMatirx = m_ProjectionMatrix * m_ViewMatrix;
	}
}