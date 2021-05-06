#include "Shader.h"
#include "Renderer.h"
#include "Karma/Core.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "Platform/Vulkan/VulkanShader.h"

namespace Karma
{
	Shader::Shader(std::shared_ptr<UniformBufferObject> ubo)
	{
		m_UniformBuffer = ubo;
	}

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
				return new VulkanShader();*/
		}

		KR_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

	Shader* Shader::Create(const std::string& vertexSrcFile, const std::string& fragmentSrcFile, std::shared_ptr<UniformBufferObject> ubo, bool bIsFile)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			KR_CORE_ASSERT(false, "RendererAPI::None is currently not supported");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return new OpenGLShader(vertexSrcFile, fragmentSrcFile, ubo, bIsFile);
			case RendererAPI::API::Vulkan:
				return new VulkanShader(vertexSrcFile, fragmentSrcFile, ubo);
		}

		KR_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}