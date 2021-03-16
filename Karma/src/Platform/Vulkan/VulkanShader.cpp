#include "VulkanShader.h"
#include "glslang/Public/ShaderLang.h"
#include "SPIRV/GlslangToSpv.h"
#include "StandAlone/DirStackFileIncluder.h"

namespace Karma
{
	VulkanShader::VulkanShader(const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		
	}

	VulkanShader::~VulkanShader()
	{
	
	}

	void VulkanShader::Bind() const
	{}

	void VulkanShader::UnBind() const
	{}

	void VulkanShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{}
}