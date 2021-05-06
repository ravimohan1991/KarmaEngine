#pragma once

#include "Karma/Core.h"
#include "Karma/Renderer/Buffer.h"
#include <string>
#include "glm/glm.hpp"

namespace Karma
{
	class KARMA_API Shader
	{
	public:
		Shader(std::shared_ptr<UniformBufferObject> ubo);
		virtual ~Shader() = default;
		
		virtual void Bind() const {}
		virtual void UnBind() const {}

		virtual void GenerateUniformBufferObject() {}
		virtual void BindUniformBufferObject() {}
		virtual void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) {}
		virtual void UploadUniformBuffer() {}

		std::shared_ptr<UniformBufferObject> GetUniformBuffer() { return m_UniformBuffer; }

		static Shader* Create(const std::string& vertexSrc, const std::string& fragmentSrc);
		static Shader* Create(const std::string& vertexSrcFile, const std::string& fragmentSrcFile, std::shared_ptr<UniformBufferObject> ubo, bool bIsFile);

	private:
		std::shared_ptr<UniformBufferObject> m_UniformBuffer;
	};
}