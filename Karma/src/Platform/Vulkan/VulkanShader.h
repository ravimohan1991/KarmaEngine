#pragma once

#include "Karma/Renderer/Shader.h"

namespace Karma
{
	class KARMA_API VulkanShader : public Shader
	{
	public:
		VulkanShader(const std::string& vertexSrc, const std::string& fragmentSrc);
		virtual ~VulkanShader() override;

		virtual void Bind() const override;
		virtual void UnBind() const override;

		virtual void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) override;
	};

}