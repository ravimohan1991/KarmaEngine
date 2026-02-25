#include "OrthographicCamera.h"
#include "KarmaRHI/DynamicRHI.h"
#include "glm/gtc/matrix_transform.hpp"
#include "Camera.h"

namespace Karma
{	
	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top) :
        Camera(glm::vec3(0.f))
	{
		m_CameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
		InitializePitchRoll();
		m_CameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
		m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_CameraFront, m_CameraUp);
        switch (GRHIInterfaceType)
		{
            case ERHIInterfaceType::Null:
                KR_CORE_ASSERT(false, "Without graphics API not supported");
				break;
            case ERHIInterfaceType::OpenGL:
				m_ProjectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
				break;
            case ERHIInterfaceType::Vulkan:
				m_ProjectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);// May want to adjust z range when supporting 3D
				m_ProjectionMatrix[1][1] *= -1.0f;
				break;
		}
	}
}
