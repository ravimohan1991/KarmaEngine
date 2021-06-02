#include "OrthographicCamera.h"
#include "Karma/Renderer/Renderer.h"
#include "glm/gtc/matrix_transform.hpp"

namespace Karma
{	
	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top)
	{
		m_ViewMatrix = glm::mat4(1.0f);
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
}