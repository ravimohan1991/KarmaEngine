#pragma once

#include "Karma/Renderer/Shader.h"

namespace Karma
{
	class KARMA_API OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc);
		virtual ~OpenGLShader() override;

		virtual void Bind() const override;
		virtual void UnBind() const override;

		virtual void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) override;

	private:
		// OpenGL's identification scheme
		uint32_t m_RendererID;
	};
}