#include "Shader.h"
#include "Renderer.h"
#include "Karma/Core.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "Platform/Vulkan/VulkanShader.h"

// PCH stuff
#include <vector>

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
			case RendererAPI::API::Vulkan:
				KR_CORE_ASSERT(false, "Creating Vulkan shader this way is not supported");
				return nullptr;// Use the overloaded version
		}

		KR_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

	Shader* Shader::Create(const std::string& vertexSrcFile, const std::string& fragmentSrcFile, std::shared_ptr<UniformBufferObject> ubo,
		const std::string& shaderName)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				KR_CORE_ASSERT(false, "RendererAPI::None is currently not supported");
				return nullptr;
			case RendererAPI::API::OpenGL:
				return new OpenGLShader(vertexSrcFile, fragmentSrcFile, ubo, shaderName);
			case RendererAPI::API::Vulkan:
				return new VulkanShader(vertexSrcFile, fragmentSrcFile, ubo);
		}

		KR_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}
