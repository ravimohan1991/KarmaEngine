#include "Shader.h"
#include "KarmaRHI/DynamicRHI.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "Platform/Vulkan/VulkanShader.h"

// PCH stuff
#include <vector>

namespace Karma
{
	Shader* Shader::Create(const std::string& vertexSrc, const std::string& fragmentSrc)
	{
        switch (GRHIInterfaceType)
		{
            case ERHIInterfaceType::Null:
                KR_CORE_ASSERT(false, "Without graphics API currently not supported");
				return nullptr;
            case ERHIInterfaceType::OpenGL:
				return new OpenGLShader(vertexSrc, fragmentSrc);
            case ERHIInterfaceType::Vulkan:
				KR_CORE_ASSERT(false, "Creating Vulkan shader this way is not supported");
				return nullptr;// Use the overloaded version
		}

		KR_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

	Shader* Shader::Create(const std::string& vertexSrcFile, const std::string& fragmentSrcFile, const std::string& shaderName)
	{
        switch (GRHIInterfaceType)
		{
            case ERHIInterfaceType::Null:
                KR_CORE_ASSERT(false, "Without graphics API currently not supported");
				return nullptr;
            case ERHIInterfaceType::OpenGL:
				return new OpenGLShader(vertexSrcFile, fragmentSrcFile, shaderName);
            case ERHIInterfaceType::Vulkan:
				return new VulkanShader(vertexSrcFile, fragmentSrcFile);
		}

		KR_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}
