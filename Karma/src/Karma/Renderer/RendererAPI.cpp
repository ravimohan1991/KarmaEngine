#include "RendererAPI.h"

namespace Karma
{
	RendererAPI::API RendererAPI::s_API = RendererAPI::API::Vulkan;
	glm::vec4 RendererAPI::m_ClearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
}
