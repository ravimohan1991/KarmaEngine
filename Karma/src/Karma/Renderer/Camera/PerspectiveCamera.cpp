#include "PerspectiveCamera.h"
#include "KarmaRHI/DynamicRHI.h"

#include "glm/gtc/matrix_transform.hpp"

namespace Karma
{
	PerspectiveCamera::PerspectiveCamera(float fovRad, float aspectRatio, float nearPlane, float farPlane) :
		Camera(glm::vec3(2.5f, 0.0f, 1.0f))
	{
		m_CameraFront = glm::vec3(-1.0f, 0.0f, 0.0f);
		InitializePitchRoll();
		m_CameraUp = glm::vec3(0.0f, 0.0f, 1.0f);
		m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_CameraFront, m_CameraUp);
        switch (GRHIInterfaceType)
		{
            case ERHIInterfaceType::Null:
                KR_CORE_ASSERT(false, "Without graphics API not supported");
				break;
            case ERHIInterfaceType::OpenGL:
				m_ProjectionMatrix = glm::perspective(glm::radians(fovRad), aspectRatio, nearPlane, farPlane);
				break;
            case ERHIInterfaceType::Vulkan:
				m_ProjectionMatrix = glm::perspective(glm::radians(fovRad), aspectRatio, nearPlane, farPlane);
				m_ProjectionMatrix[1][1] *= -1.0f;
				break;
		}
	}
}
