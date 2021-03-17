#pragma once

#include "Karma/Renderer/Shader.h"
#include "glad/glad.h"
#include <string>
#include <map>

namespace Karma
{
	class KARMA_API OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc);
		OpenGLShader(const std::string& vertexSrcFile, const std::string& fragmentSrcFile, bool bIsFile);
		virtual ~OpenGLShader() override;

		virtual void Bind() const override;
		virtual void UnBind() const override;

		virtual void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) override;

	private:
		std::string ReadFile(const std::string& file);
		void Compile(const std::unordered_map<GLenum, std::string>& shaderSources);

	private:
		// OpenGL's identification scheme
		uint32_t m_RendererID;
	};
}