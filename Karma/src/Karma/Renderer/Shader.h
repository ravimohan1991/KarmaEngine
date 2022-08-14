#pragma once

#include "krpch.h"

#include "Karma/Renderer/Buffer.h"
#include "glm/glm.hpp"

namespace Karma
{
	class KARMA_API Shader
	{
	public:
		Shader(std::shared_ptr<UniformBufferObject> ubo) : m_UniformBufferObject(ubo)
		{}
		virtual ~Shader() = default;
		
		virtual void Bind() const {}
		virtual void Bind(const std::string& texShaderNames) const {}
		virtual void UnBind() const {}

		virtual void GenerateUniformBufferObject() {}
		virtual void BindUniformBufferObject() {}
		std::shared_ptr<UniformBufferObject> GetUniformBufferObject() const { return m_UniformBufferObject; }

		static Shader* Create(const std::string& vertexSrc, const std::string& fragmentSrc);
		static Shader* Create(const std::string& vertexSrcFile, const std::string& fragmentSrcFile, std::shared_ptr<UniformBufferObject> ubo,
			bool bIsFile, const std::string& shaderName = "NoNamedShader");

		// Getters
		const std::string& GetShaderName() const { return m_ShaderName; }

	private:
		std::shared_ptr<UniformBufferObject> m_UniformBufferObject;
	
	protected:
		std::string m_ShaderName;
	};
}
