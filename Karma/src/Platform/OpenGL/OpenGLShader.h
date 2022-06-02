#pragma once

#include "Karma/Renderer/Shader.h"
#include "glad/glad.h"
#include <string>
#include <map>

namespace Karma
{
	struct OpenGLUniformBuffer;
	class KARMA_API OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc);
		OpenGLShader(const std::string& vertexSrcFile, const std::string& fragmentSrcFile, std::shared_ptr<UniformBufferObject> ubo, bool bIsFile, 
			const std::string& shaderName);
		virtual ~OpenGLShader() override;

		virtual void Bind() const override;
		virtual void Bind(const std::string& texShaderNames) const override;
		virtual void UnBind() const override;

		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);

	private:
		void Compile(const std::unordered_map<GLenum, std::string>& shaderSources);

	private:
		// OpenGL's identification scheme
		uint32_t m_RendererID;
		std::shared_ptr<OpenGLUniformBuffer> m_UniformBufferObject;
	};
}