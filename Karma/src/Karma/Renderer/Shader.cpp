#include "Shader.h"
#include "Renderer.h"
#include "Karma/Core.h"
#include "Platform/OpenGL/OpenGLShader.h"
//#include "Platform/Vulkan/VulkanVertexArray.h"

namespace Karma
{
	Shader* Shader::Create(const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				KR_CORE_ASSERT(false, "RendererAPI::None is currently not supported");
				return nullptr;
			case RendererAPI::API::OpenGL:
				return new OpenGLShader(vertexSrc, fragmentSrc);
			/*case RendererAPI::API::Vulkan:
				return new VulkanVertexArray();*/
		}

		KR_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}